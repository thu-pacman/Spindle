#include "STracer.h"
#include "instrument.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/FileSystem.h"
#include "utils.h"
#include "visitor.h"

using namespace llvm;

namespace {

cl::opt<bool> fullMem("full_mem", cl::init(false));
cl::opt<bool> fullBr("full_br", cl::init(false));
cl::opt<bool> replay("replay", cl::init(false));

class STracerPass : public PassInfoMixin<STracerPass> {
public:
    static void getAnalysisUsage(AnalysisUsage &AU) {
        // invoke `loopSimplify` pass before STracerPass
        AU.addRequiredID(LoopSimplifyID);
        AU.setPreservesCFG();
    }

    static PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
        // show args
        errs() << "Args:\n";
        errs() << "\tfull_mem: " << fullMem << '\n';
        errs() << "\tfull_br: " << fullBr << '\n';
        errs() << "\treplay: " << replay << '\n';

        // preprocess(M);  // to expand nested_GEPInst

        MASModule MAS(M);
        MAS.analyze();
        auto *instrument =
            replay ? new InstrumentationBase(M) : new Instrumentation(M);
        STracer stracer(MAS);
        stracer.run(instrument, fullMem, fullBr);
        if (!replay) {
            // instrument for main function
            if (auto main = M.getFunction("main")) {
                // init main
                instrument->init_main(&main->getEntryBlock().front());
                // fini main
                for (auto &BB : *main) {
                    if (auto RetI = dyn_cast<ReturnInst>(BB.getTerminator())) {
                        instrument->fini_main(RetI);
                    }
                }
                for (auto &F : M) {
                    for (auto &BB : F) {
                        for (auto &I : BB) {
                            if (auto CallI = dyn_cast<CallInst>(&I)) {
                                auto func = CallI->getCalledFunction();
                                if (func && func->getName().equals("exit")) {
                                    instrument->fini_main(CallI);
                                }
                            }
                        }
                    }
                }
            }
            PreservedAnalyses PA = PreservedAnalyses::none();
            PA.preserveSet<CFGAnalyses>();
            return PA;
        } else {
            std::error_code ec;
            raw_fd_ostream full_trace("full_trace.log", ec);
            DTraceParser dTraceParser("dtrace.log");
            SymbolTable table;
            stracer.replay(M.getFunction("main"),
                           dTraceParser,
                           full_trace,
                           instrument->getInstrumentedSymbols(),
                           table);
            return PreservedAnalyses::all();
        }
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
