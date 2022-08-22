#include "STracer.h"

#include <queue>

using namespace llvm;

namespace llvm {

void STracer::run(Instrumentation &instrument) {
    std::error_code ec;
    raw_fd_ostream strace("strace.log", ec);
    int tot = 0, cnt = 0;
    for (auto F : MAS.functions) {
        strace << "Function: " << F->func.getName() << "\n";
        // step 1: find GEP dependencies
        auto depVisitor = GEPDependenceVisitor(F->instrMeta, F->indVars);
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                // TODO: Nesting GEP in other instructions
                if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    depVisitor.visit(*GEPI);
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
        for (; !q.empty(); q.pop()) {
            for (auto BB : successors(q.front())) {
                if (!F->bbMeta[BB].needRecord) {
                    F->bbMeta[BB].needRecord = true;
                    q.push(BB);
                }
            }
        }
        for (auto &BB : F->func) {
            if (!F->bbMeta[&BB].inMASLoop && F->bbMeta[&BB].needRecord) {
                if (auto BrI = dyn_cast<BranchInst>(BB.getTerminator());
                    BrI && BrI->isConditional()) {  // instrument for br
                    F->instrMeta[BrI].isSTraceDependence = true;
                    instrument.record_br(BrI);
                }
            }
        }
        // step 3: print static trace
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                auto loop = F->instrMeta[&I].loop;
                if (loop && isa<PHINode>(I)) {  // TODO: add loop end position
                    strace << "  For loop starts at " << I << '\n';
                    for (auto &indVar : loop->indVars) {
                        if (auto def =
                                dyn_cast<Instruction>(indVar.initValue)) {
                            instrument.record_value(def);
                        }
                        if (auto def =
                                dyn_cast<Instruction>(indVar.finalValue)) {
                            instrument.record_value(def);
                        }
                        strace << "\tLoop IndVar starts from "
                               << *indVar.initValue << ", ends at "
                               << *indVar.finalValue << ", steps by ";
                        indVar.delta->print(strace);
                        strace << '\n';
                    }
                    if (auto endPosition = loop->getEndPosition()) {
                        strace << "  For loop ends at " << *endPosition << '\n';
                    }
                } else if (F->instrMeta[&I].isSTraceDependence) {
                    strace << I << '\n';
                } else if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    strace << *GEPI << "\n\tFormula: ";
                    auto visitor = loop ? ASTVisitor([&](Value *v) {
                        return loop->isLoopInvariant(v) || F->indVars.count(v);
                    })
                                        : ASTVisitor([](Value *v) {
                                              return Constant::classof(v) ||
                                                     Argument::classof(v);
                                          });
                    auto formula = visitor.visit(GEPI);
                    formula->print(strace);
                    strace << '\n';
                    if (loop) {
                        ++tot;
                        cnt += formula->computable;
                    }
                    /*
                    if (!formula->computable && loop) {
                        errs() << *GEPI << '\n';
                        formula->print(errs());
                        errs() << '\n';
                    }*/
                    if (!formula->computable) {
                        instrument.record_value(GEPI);
                    }  // TODO: instrument for loop invariants
                }
            }
        }
    }
    errs() << "Computable memory accesses in loops: " << cnt << '/' << tot
           << '\n';
    errs() << "Static trace has been dumped into strace.log.\n";
}

}  // namespace llvm