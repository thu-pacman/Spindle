#include "MTS.h"
#include "SDetector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <fstream>

using namespace llvm;

namespace {

class SpindlePass : public PassInfoMixin<SpindlePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    errs() << "Begin: " << M.getModuleIdentifier() << "\n";
    auto MMC = MyModuleContext(M);
    auto i = 0;
    for (auto &F : M) {
      errs() << "Function " << i++ << ": " << F.getName() << "\n";
      // TODO: Add lib function.
      if (F.isDeclaration())
        continue;
      analysisFunction(F, MMC);
    }

    auto SD = SDetector(MMC);

    for (auto MF : MMC) {
      MF->applyInstrumentations();
    }

    // Instrument for main function.
    errs() << "Instrumenting for main\n";
    auto main = M.getFunction("main");
    if (main) {
      {
        IRBuilder<> Builder(&(*main->begin()->begin()));
        vector<Type *> argsType;
        ArrayRef<Type *> argsRef(argsType);
        auto funcType = FunctionType::get(Builder.getVoidTy(), argsRef, false);
        auto initFunc = M.getOrInsertFunction("__init_main", funcType);
        Builder.CreateCall(initFunc);
      }
      for (auto & bb : *main) {
        for (auto inst = bb.begin(), iend = bb.end(); inst != iend; ++inst) {
          if (auto ri = dyn_cast<ReturnInst>(inst)) {
            IRBuilder<> Builder(&(*inst));
            vector<Type *> argsType;
            ArrayRef<Type *> argsRef(argsType);
            auto funcType =
                FunctionType::get(Builder.getVoidTy(), argsRef, false);
            auto finiFunc = M.getOrInsertFunction("__fini_main", funcType);
            Builder.CreateCall(finiFunc);
          } else if (auto ci = dyn_cast<CallInst>(inst)) {
            // auto fn = ci->getCalledFunction();
            // if (!fn)
            //   continue;
            // if (fn->getName().equals("exit")) {
            //   IRBuilder<> Builder(&(*inst));
            //   vector<Type *> argsType;
            //   ArrayRef<Type *> argsRef(argsType);
            //   auto funcType =
            //       FunctionType::get(Builder.getVoidTy(), argsRef, false);
            //   auto finiFunc = M.getOrInsertFunction("__fini_main", funcType);
            //   Builder.CreateCall(finiFunc);
            // }
          }
        }
      }
    }

    for (auto &F : M) {
      for (auto &BB : F) {
        for (auto &inst : BB) {
          if (auto ci = dyn_cast<CallInst>(&inst)) {
            auto fn = ci->getCalledFunction();
            if (!fn)
              continue;
            if (fn->getName().equals("exit")) {
              IRBuilder<> Builder(&inst);
              vector<Type *> argsType;
              ArrayRef<Type *> argsRef(argsType);
              auto funcType =
                  FunctionType::get(Builder.getVoidTy(), argsRef, false);
              auto finiFunc = M.getOrInsertFunction("__fini_main", funcType);
              Builder.CreateCall(finiFunc);
            }
          }
        }
      }
    }

    writeStaticTrace(MMC);

    // TODO: Delete all the new objects.
    errs() << "End\n";
    return PreservedAnalyses::none();
  }

  void analysisFunction(Function &F, MyModuleContext &MMC) {
    auto MF = new MyFunction(F, MMC);
    // for (auto &MBB : MF.getBBs()) {
    //     MBB.getBB().print(errs());
    // }
    // for (auto &BB : F) {
    // 	BB.print(errs());
    // }

    // DFS to traverse all the loops in a specific function.
    // TODO: Add this part to MyFunction.
    PassBuilder PB;
    FunctionAnalysisManager FAM;
    PB.registerFunctionAnalyses(FAM);
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    vector<Loop *> workspace;
    for (auto it : LI) {
      workspace.push_back(it);
    }
    while (!workspace.empty()) {
      auto L = workspace.back();
      workspace.pop_back();
      MF->addLoop(*L);
      for (auto sl : *L) {
        workspace.push_back(sl);
      }
    }

    // Update LoopID for MyBasicBlocks.
    // TODO: Add this part to MyFunction.
    auto &MLs = MF->getLoops();
    for (auto MBB : *MF) {
      auto L = LI.getLoopFor(&MBB->getBB());
      if (!L)
        continue;
      for (auto ML : MLs) {
        if (L == &ML->getLoop()) {
          MBB->setLoopID(ML->getID());
          break;
        }
      }
    }

    MF->loopAnalysis();
  }

  void writeStaticTrace(MyModuleContext &MMC) {
    errs() << "Writing static trace\n";
    ofstream out;
    out.open(MMC.getModuleName() + ".st", ios::in | ios::out);
    auto &FL = MMC.getFunctionList();
    for (auto MF : FL) {
      auto fid = MF->getID();
      out << "Function"
          << " " << fid << " " << MF->getNumArgs();
    }
    out.close();
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
