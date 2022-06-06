#include "STracer.h"

#include <queue>

using namespace llvm;

namespace llvm {

void STracer::run(const Instrumentation &instrument) {
    errs() << "Static trace:\n";
    for (auto F : MAS.getFunctions()) {
        errs() << " Function: " << F->func.getName() << "\n";
        // step 1: find GEP dependencies
        auto visitor = GEPDependenceVisitor(F->instrMeta, F->indVars);
        for (auto &BB : F->func) {
            for (auto &I : BB) {
                // TODO: Nesting GEP in other instructions
                if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    visitor.visit(*GEPI);
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
            if (!F->bbMeta[&BB].inLoop && F->bbMeta[&BB].needRecord) {
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
                if (auto loop = F->instrMeta[&I].loop) {
                    errs() << "  For loop starts at " << I << '\n';
                    for (auto &indVar : loop->indVars) {
                        errs() << "\tLoop IndVar start from "
                               << *indVar.initValue << ", step by ";
                        indVar.delta->print();
                        errs() << '\n';
                    }
                } else if (F->instrMeta[&I].isSTraceDependence) {
                    errs() << I << '\n';
                } else if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    errs() << *GEPI << "\n\tFormula: ";
                    auto *formula =
                        ASTVisitor([&](Value *v) {
                            return Constant::classof(v) ||
                                   F->indVars.find(v) != F->indVars.end();
                        }).visit(GEPI);
                    formula->print();
                    errs() << '\n';
                }
            }
        }
    }
}

}  // namespace llvm
