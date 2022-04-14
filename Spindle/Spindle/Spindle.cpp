#include "STracer.h"
#include "visitor.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <unordered_set>

using namespace llvm;

namespace {

class SpindlePass : public PassInfoMixin<SpindlePass> {
    MASModule MAS;

  public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
        MAS.analyze(M);
        STracer(MAS).print();
        return PreservedAnalyses::none();
    }

};

} // End of anonymous namespace.

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "SpindlePass", "v0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                    [](StringRef PassName, ModulePassManager &MPM, ...) {
                        if (PassName == "spindle") {
                            MPM.addPass(SpindlePass());
                            return true;
                        }
                        return false;
                    });
        }
    };
}
