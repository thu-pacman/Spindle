#include "STracer.h"

#include <queue>
#include <ranges>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "utils.h"

using namespace llvm;

namespace llvm {

DTraceParser::DTraceParser(const std::string &fname) : dtrace(fname) {
}

auto DTraceParser::parseBr() -> bool {
    std::string line;
    std::getline(dtrace, line);
    assert(line[0] == 'b');
    errs() << line << '\n';
    return line[1] - '0';
}

auto DTraceParser::parseValue() -> ValueType {
    std::string line;
    std::getline(dtrace, line);
    assert(line[0] == 'v');
    errs() << line << '\n';
    return std::stoull(line.substr(1));
}

STracer::STracer(MASModule &MAS) : MAS(MAS) {
}

void STracer::run(InstrumentationBase *instrument, bool fullMem, bool fullBr) {
    int tot = 0, cnt = 0;
    for (auto F : MAS.functions) {
        // step 1: find memory access dependencies
        auto depVisitor = MemDependenceVisitor(
            F->instrMeta,
            F->indVars);  // mark all dependent instructions in `instrMeta`
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                if (auto LI = dyn_cast<LoadInst>(&I)) {
                    depVisitor.visit(LI);
                } else if (auto SI = dyn_cast<StoreInst>(&I)) {
                    depVisitor.visit(SI);
                }
            }
        }
        // step 2: for each BB find whether the branch should be recorded
        std::queue<BasicBlock *> q;
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                if (isa<GetElementPtrInst>(I) ||
                    F->instrMeta[&I].isSTraceDependence) {
                    F->bbMeta[&BB].needRecord = true;
                    q.push(&BB);
                    break;
                }
            }
        }
        for (; !q.empty(); q.pop()) {  // all the needed-record BB's succeeding
                                       // BBs need to be recorded
            for (auto BB : successors(q.front())) {
                if (!F->bbMeta[BB].needRecord) {
                    F->bbMeta[BB].needRecord = true;
                    q.push(BB);
                }
            }
        }
        for (auto &BB : F->func) {
            if (!F->bbMeta[&BB].inMASLoop && F->bbMeta[&BB].needRecord ||
                fullBr) {
                if (auto BrI = dyn_cast<BranchInst>(BB.getTerminator());
                    BrI && BrI->isConditional()) {  // instrument for br
                    F->instrMeta[BrI].isSTraceDependence = true;
                    instrument->record_br(BrI);
                }
            }
        }
        // step 3: record static trace
        for (auto &BB : F->func) {
            auto loop = F->bbMeta[&BB].loop;
            if (loop) {
                for (auto &indVar : loop->indVars) {
                    if (auto def = dyn_cast<Instruction>(indVar.initValue)) {
                        instrument->record_value(def);
                    }
                }
            }
            for (auto &I : BB) {
                if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
                    auto ptr = getPointerOperand(&I);
                    auto visitor =
                        loop
                            ? FormulaVisitor(
                                  [&](Value *v) {
                                      return loop->isLoopInvariant(v) ||
                                             F->indVars.count(v);
                                  },
                                  &MAS)
                            : FormulaVisitor(
                                  [](Value *v) { return isa<ConstantData>(v); },
                                  &MAS);
                    auto formula = visitor.visitValue(ptr);
                    if (loop && !fullMem) {
                        ++tot;
                        if (formula->computable) {
                            ++cnt;
                            F->instrMeta[&I].formula = formula;
                            InstrumentationVisitor([&](ASTLeafNode *v) {
                                if (!isa<ConstantData>(v->v) &&
                                    loop->isLoopInvariant(v->v)) {
                                    instrument->record_value(v->v);
                                }
                            }).dispatch(formula);
                            // TODO: prune instrumentation for parent loop
                            // invariant (nested_loop.c)
                        }
                    }
                    if (!formula->computable || fullMem) {
                        instrument->record_value(ptr);
                    }
                }
            }
        }
    }
    if (!fullMem) {
        errs() << "Computable loops: " << MAS.num_computable_loops << '/'
               << MAS.num_loops << '\n';
        errs() << "Computable memory accesses in loops: " << cnt << '/' << tot
               << '\n';
    }
}

void STracer::replay(Function *func,
                     DTraceParser &dtrace,
                     raw_fd_ostream &out,
                     InstrumentationBase *instrument,
                     SymbolTable &table) {
    auto &instrumentedSymbols = instrument->getInstrumentedSymbols();
    // deal with globals and arguments
    for (auto instrumentedGlobal : std::ranges::reverse_view(
             instrument->getInstrumentedArgs(func->getName()))) {
        table[instrumentedGlobal] = dtrace.parseValue();
    }
    MASFunction *MASFunc;
    for (auto f : MAS.functions) {
        if (&f->func == func) {
            MASFunc = f;
        }
    }
    for (auto *curBB = &func->getEntryBlock();;) {
        for (auto &I : *curBB) {
            if (isa<ReturnInst>(I)) {
                return;
            }
            // reach the end of basic block
            if (auto BrI = dyn_cast<BranchInst>(&I)) {
                if (BrI->isConditional()) {
                    curBB = BrI->getSuccessor(!dtrace.parseBr());
                } else {
                    curBB = BrI->getSuccessor(0);
                }
                break;
            }
            if (auto CallI = dyn_cast<CallInst>(&I)) {
                auto calledFunc = CallI->getCalledFunction();
                if (!calledFunc->isDeclaration()) {
                    replay(calledFunc, dtrace, out, instrument, table);
                }
            }
            if (instrumentedSymbols.find(&I) != instrumentedSymbols.end()) {
                table[&I] = dtrace.parseValue();
            }
            // update induction variable
            if (auto indVar = MASFunc->instrMeta[&I].indVar) {
                if (table.find(&I) == table.end()) {  // init value
                    auto initDef = dyn_cast<Instruction>(indVar->initValue);
                    if (instrumentedSymbols.find(initDef) !=
                        instrumentedSymbols.end()) {
                        table[&I] = table[initDef];
                    } else {
                        table[&I] = cast<ConstantInt>(indVar->initValue)
                                        ->getZExtValue();
                    }
                } else {
                    table[&I] =
                        CalculationVisitor(table).dispatch(indVar->delta);
                }
            }
            // replay mem trace
            if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
                auto ptr = getPointerOperand(&I);
                if (instrumentedSymbols.find(ptr) !=
                    instrumentedSymbols.end()) {
                    out << table[ptr] << '\n';
                } else {
                    out << CalculationVisitor(table).dispatch(
                               MASFunc->instrMeta[&I].formula)
                        << '\n';
                }
            }
        }
    }
}

}  // namespace llvm
