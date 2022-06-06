#include "STracer.h"
#include "llvm/IR/IRBuilder.h"
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
        // STracer(MAS).print();
        // Instrument for main function
        if (auto main = M.getFunction("main")) {
            // init main
            {
                IRBuilder builder(&*main->begin()->begin());
                auto funcType =
                    FunctionType::get(builder.getVoidTy(), {}, false);
                auto initFunc =
                    M.getOrInsertFunction("__spindle_init_main", funcType);
                builder.CreateCall(initFunc);
            }
            // fini main
            for (auto &BB : *main) {
                if (auto ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
                    IRBuilder builder(&*ret);
                    auto funcType =
                        FunctionType::get(builder.getVoidTy(), {}, false);
                    auto finiFunc =
                        M.getOrInsertFunction("__spindle_fini_main", funcType);
                    builder.CreateCall(finiFunc);
                    break;
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
