#pragma once
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

class Instrumentation {
    Module &M;
    set<Instruction *> valueRecorded;

public:
    explicit Instrumentation(Module &M) : M(M) {
    }
    StringRef getName() const {
        return M.getName();
    }
    void init_main(Instruction *I) const {
        IRBuilder builder(I);
        auto type = FunctionType::get(builder.getVoidTy(), {}, false);
        auto func = M.getOrInsertFunction("__spindle_init_main", type);
        builder.CreateCall(func);
    }
    void fini_main(Instruction *I) const {
        IRBuilder builder(I);
        auto type = FunctionType::get(builder.getVoidTy(), {}, false);
        auto func = M.getOrInsertFunction("__spindle_fini_main", type);
        builder.CreateCall(func);
    };
    void record_br(BranchInst *I) const {
        IRBuilder builder(I);
        auto type = FunctionType::get(
            builder.getVoidTy(), {I->getCondition()->getType()}, false);
        auto func = M.getOrInsertFunction("__spindle_record_br", type);
        builder.CreateCall(func, {I->getCondition()});
    }
    void record_value(Instruction *I) {
        if (valueRecorded.count(I)) {
            return;
        }
        valueRecorded.insert(I);
        auto nextNonPhi = I->getNextNode();
        for (; nextNonPhi != nullptr && isa<PHINode>(nextNonPhi);
             nextNonPhi = nextNonPhi->getNextNode());
        IRBuilder builder(nextNonPhi);
        auto value = cast<Value>(I);
        auto type =
            FunctionType::get(builder.getVoidTy(), {value->getType()}, false);
        auto func = M.getOrInsertFunction("__spindle_record_value", type);
        builder.CreateCall(func, {value});
    }
};
