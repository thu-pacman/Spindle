#include "STracer.h"

using namespace llvm;

namespace llvm {

void STracer::print() {
    errs() << "Static trace:\n";
    for (auto F : MAS.getFunctions()) {
        auto &rawF = F->func;
        errs() << " Function: " << rawF.getName() << "\n";
        auto indVars = F->indVars;
        auto meta = F->meta;
        for (auto &BB: rawF) {
            for (auto &I : BB) {
                if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    GEPDependenceVisitor(meta, indVars).visit(*GEPI);
                }
            }
        }
        for (auto &BB : rawF) {
            for (auto &I : BB) {
                if (auto loop = meta[&I].loop) {
                    errs() << "  For loop starts at " << I << '\n';
                    for (auto &indVar : loop->indVars) {
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
