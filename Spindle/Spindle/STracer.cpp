#include "STracer.h"

using namespace llvm;

namespace llvm {

void STracer::print() {
    int tot = 0, computable = 0;
    for (auto F: MAS.getFunctions()) {
        auto indVars = F->getIndVarSet();
        for (auto &BB : F->getRawFunction()) {
            for (auto &I : BB) {
                if (auto GEPI = dyn_cast<GetElementPtrInst>(&I)) {
                    auto *formula = ASTVisitor(
                            [&](Value *v) {
                                return Constant::classof(v) ||
                                       indVars.find(v) != indVars.end();
                            }).visit(GEPI);
                    ++tot;
                    computable += formula->computable;
                }
            }
        }
    }
    errs() << "MemAccess analyzed: " << computable << '/' << tot << " computable\n";
}

} // namespace llvm
