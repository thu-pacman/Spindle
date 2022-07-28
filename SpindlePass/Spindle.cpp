#include "STracer.h"
#include "instrument.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "visitor.h"

using namespace llvm;

namespace {

class SpindlePass : public PassInfoMixin<SpindlePass> {
    MASModule MAS;

public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
        MAS.analyze(M);
        Instrumentation instrument(M);
        STracer(MAS).run(instrument);
        // instrument for main function
        if (auto main = M.getFunction("main")) {
            // init main
            instrument.init_main(&main->getEntryBlock().front());
            // fini main
            for (auto &BB : *main) {
                if (auto RetI = dyn_cast<ReturnInst>(BB.getTerminator())) {
                    instrument.fini_main(RetI);
                }
            }
            for (auto &F : M) {
                for (auto &BB : F) {
                    for (auto &I : BB) {
                        if (auto CallI = dyn_cast<CallInst>(&I)) {
                            auto func = CallI->getCalledFunction();
                            if (func && func->getName().equals("exit")) {
                                instrument.fini_main(CallI);
                            }
                        }
                    }
                }
            }
        }
        return PreservedAnalyses::none();
    }
};

}  // End of anonymous namespace.

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "SpindlePass", "v0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef PassName, ModulePassManager &MPM, ...) {
                    if (PassName == "spindle") {
                        MPM.addPass(SpindlePass());
                        return true;
                    }
                    return false;
                });
        }};
}