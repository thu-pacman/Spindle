#include "STracer.h"

#include <queue>

#include "llvm/Support/FileSystem.h"
#include "utils.h"

using namespace llvm;

namespace llvm {

void STracer::run(Instrumentation &instrument, bool fullMem, bool fullBr) {
    std::error_code ec;
    raw_fd_ostream strace("strace.log", ec, sys::fs::OF_Append);
    strace << "File: " << instrument.getName() << "\n";
    int tot = 0, cnt = 0;
    for (auto F : MAS.functions) {
        strace << "Function: " << F->func.getName() << "\n";
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
        queue<BasicBlock *> q;
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                if (dyn_cast<GetElementPtrInst>(&I) ||
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
                    instrument.record_br(BrI);
                }
            }
        }
        // step 3: print static trace
        for (auto &BB : F->func) {
            auto loop = F->bbMeta[&BB].loop;
            if (loop) {
                strace << "  For loop with header " << BB.getName() << '\n';
                for (auto &indVar : loop->indVars) {
                    if (auto def = dyn_cast<Instruction>(indVar.initValue)) {
                        instrument.record_value(def);
                    }
                    if (auto def = dyn_cast<Instruction>(indVar.finalValue)) {
                        instrument.record_value(def);
                    }
                    strace << "\tLoop IndVar starts from " << *indVar.initValue
                           << ", ends at " << *indVar.finalValue
                           << ", steps by ";
                    indVar.delta->print(strace);
                    strace << '\n';
                }
            }
            for (auto &I : BB) {
                Instruction *ptr = nullptr;
                auto LI = dyn_cast<LoadInst>(&I);
                if (LI) {
                    ptr = dyn_cast<Instruction>(LI->getPointerOperand());
                }
                auto SI = dyn_cast<StoreInst>(&I);
                if (SI) {
                    ptr = dyn_cast<Instruction>(SI->getPointerOperand());
                }
                if (ptr) {
                    if (LI) {
                        strace << *LI;
                    } else {
                        strace << *SI;
                    }
                    strace << "\n\tFormula: ";
                    auto visitor = loop ? ASTVisitor([&](Value *v) {
                        return loop->isLoopInvariant(v) || F->indVars.count(v);
                    })
                                        : ASTVisitor([](Value *v) {
                                              return Constant::classof(v);
                                          });
                    auto formula = visitor.visit(ptr);
                    formula->print(strace);
                    strace << '\n';
                    if (loop) {
                        ++tot;
                        if (formula->computable) {
                            ++cnt;
                            InstrumentationVisitor([&](ASTLeafNode *v) {
                                auto def = dyn_cast<Instruction>(v->v);
                                if (def && loop->isLoopInvariant(v->v)) {
                                    instrument.record_value(def);
                                }
                            }).visit(formula);
                        }
                    }
                    /*
                    if (!formula->computable && loop) {
                        errs() << "non-computable in loop:\n";
                        if (LI) {
                            errs() << '\t' << *LI << "\n\t";
                        } else {
                            errs() << '\t' << *SI << "\n\t";
                        }
                        formula->print(errs());
                        errs() << '\n';
                        ASTVisitor(
                            [&](Value *v) {
                              return loop->isLoopInvariant(v) ||
                                     F->indVars.count(v);
                            },
                            true)
                            .visit(ptr);
                    }
                     */
                    if (!formula->computable || fullMem) {
                        instrument.record_value(ptr);
                    }  // TODO: instrument for loop invariants
                }
            }
        }
    }
    errs() << "Computable loops: " << MAS.num_computable_loops << '/'
           << MAS.num_loops << '\n';
    errs() << "Computable memory accesses in loops: " << cnt << '/' << tot
           << '\n';
    errs() << "Static trace has been dumped into " << STRACE_FILE_NAME << ".\n";
}

}  // namespace llvm
