#pragma once
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

class Instrumentation {
    Module &M;

public:
    explicit Instrumentation(Module &M) : M(M) {
    }
    void init_main(Instruction *I) const {
        IRBuilder builder(I);
        auto funcType = FunctionType::get(builder.getVoidTy(), {}, false);
        auto finiFunc = M.getOrInsertFunction("__spindle_init_main", funcType);
        builder.CreateCall(finiFunc);
    }
    void fini_main(Instruction *I) const {
        IRBuilder builder(I);
        auto funcType = FunctionType::get(builder.getVoidTy(), {}, false);
        auto finiFunc = M.getOrInsertFunction("__spindle_fini_main", funcType);
        builder.CreateCall(finiFunc);
    };
    void record_br(BranchInst *I) const {
        IRBuilder builder(I);
        auto funcType = FunctionType::get(
            builder.getVoidTy(), {I->getCondition()->getType()}, false);
        auto finiFunc = M.getOrInsertFunction("__spindle_record_br", funcType);
        builder.CreateCall(finiFunc, {I->getCondition()});
    }
};
