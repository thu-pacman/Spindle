#include "STracer.h"

using namespace llvm;

namespace llvm {

void STracer::print() {
    for (auto F : MAS.getFunctions()) {
        auto &rawF = F->getRawFunction();
        errs() << "Entering function: " << rawF.getName() << "\n";
        auto indVars = F->getIndVarSet();
        auto meta = F->getMeta();
        auto visitor = GEPDependenceVisitor(meta, indVars);
        errs() << "MemAccess formula:\n";
        for (auto &BB : rawF) {
            for (auto &I : BB) {
                if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    visitor.visit(*GEPI);
                    auto *formula = ASTVisitor([&](Value *v) {
                                        return Constant::classof(v) ||
                                               indVars.find(v) != indVars.end();
                                    }).visit(GEPI);
                    formula->print();
                    errs() << '\n';
                }
            }
        }
        errs() << "static trace:\n";
        for (auto &BB : F->getRawFunction()) {
            for (auto &I : BB) {
                if (dyn_cast<GetElementPtrInst>(&I) ||
                    meta[&I].isGEPDependence) {
                    errs() << I << '\n';
                }
            }
        }
    }
}

}  // namespace llvm
