#include "STracer.h"

using namespace llvm;

namespace llvm {

void STracer::print() {
    errs() << "Static trace:\n";
    for (auto F : MAS.getFunctions()) {
        auto &rawF = F->getRawFunction();
        errs() << " Function: " << rawF.getName() << "\n";
        auto indVars = F->getIndVarSet();
        auto meta = F->getMeta();
        auto visitor = GEPDependenceVisitor(meta, indVars);
        for (auto &BB: rawF) {
            for (auto &I : BB) {
                if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    visitor.visit(*GEPI);
                }
            }
        }
        for (auto &BB : rawF) {
            for (auto &I : BB) {
                if (auto loop = meta[&I].loop) {
                    errs() << "  For loop:\n";
                    for (auto &indVar : loop->getIndVars()) {
                        errs() << "\tLoop IndVar start from "
                               << *indVar.initValue << ", step by ";
                        indVar.delta->print();
                        errs() << '\n';
                    }
                } else if (meta[&I].isGEPDependence) {
                    errs() << I << '\n';
                } else if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    errs() << *GEPI << "\n\tFormula: ";
                    auto *formula = ASTVisitor([&](Value *v) {
                                        return Constant::classof(v) ||
                                               indVars.find(v) != indVars.end();
                                    }).visit(GEPI);
                    formula->print();
                    errs() << '\n';
                }
            }
        }
    }
}

}  // namespace llvm
