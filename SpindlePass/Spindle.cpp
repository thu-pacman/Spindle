#include "STracer.h"
#include "instrument.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils.h"
#include "utils.h"
#include "visitor.h"

using namespace llvm;

namespace {

cl::opt<bool> fullBr("full_br", cl::init(false));

class STracerPass : public PassInfoMixin<STracerPass> {
    MASModule MAS;

public:
    void getAnalysisUsage(AnalysisUsage &AU) const {
        // invoke `loopSimplify` pass before STracerPass
        AU.addRequiredID(LoopSimplifyID);
    }

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
        // show args
        errs() << "Args:\n";
        errs() << "\tfull_br: " << fullBr << '\n';

        preprocess(M);  // to expand nested_GEPInst

        MAS.analyze(M);
        Instrumentation instrument(M);
        STracer(MAS).run(instrument, fullBr);
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
                            if (func && func->getName().equals(
                                            "exit")) {  // may call exit()
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

llvm::PassPluginLibraryInfo getSpindlePassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "SpindlePass", "v0.1", [](PassBuilder &PB) {
            PB.registerOptimizerLastEPCallback([](ModulePassManager &MPM, ...) {
                MPM.addPass(createModuleToFunctionPassAdaptor(
                    LoopSimplifyPass()));  // add `-loop-simplify` pass
                MPM.addPass(STracerPass());
                return true;
            });
        }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getSpindlePassPluginInfo();
}
