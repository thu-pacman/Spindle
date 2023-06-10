#pragma once
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

class InstrumentationBase {
protected:
    Module &M;
    std::set<Instruction *> valueRecorded;

public:
    explicit InstrumentationBase(Module &M) : M(M) {
    }

    [[nodiscard]] auto getName() const {
        return M.getName();
    }
    [[nodiscard]] auto &getInstrumentedSymbols() {
        return valueRecorded;
    }
    virtual void init_main(Instruction *I) const {
    }
    virtual void fini_main(Instruction *I) const {
    }
    virtual void record_br(BranchInst *I) const {
    }
    virtual void record_value(Instruction *I) {
        valueRecorded.insert(I);
    }
};

class Instrumentation : public InstrumentationBase {
public:
    explicit Instrumentation(Module &M) : InstrumentationBase(M) {
    }
    void init_main(Instruction *I) const override {
        IRBuilder builder(I);
        auto type = FunctionType::get(builder.getVoidTy(), {}, false);
        auto func = M.getOrInsertFunction("__spindle_init_main", type);
        builder.CreateCall(func);
    }
    void fini_main(Instruction *I) const override {
        IRBuilder builder(I);
        auto type = FunctionType::get(builder.getVoidTy(), {}, false);
        auto func = M.getOrInsertFunction("__spindle_fini_main", type);
        builder.CreateCall(func);
    };
    void record_br(BranchInst *I) const override {
        IRBuilder builder(I);  //  created instructions should be inserted
                               //  *before* the specified instruction.
        auto type =
            FunctionType::get(builder.getVoidTy(),
                              {I->getCondition()->getType()},
                              false);  // isVarArg: indicate that the number of
                                       // arguments are variable
        auto func = M.getOrInsertFunction("__spindle_record_br", type);
        builder.CreateCall(func, {I->getCondition()});
    }
    void record_value(Instruction *I) override {
        if (valueRecorded.count(I)) {
            return;
        }
        valueRecorded.insert(I);
        auto nextNonPhi = I->getNextNode();
        for (; nextNonPhi != nullptr && isa<PHINode>(nextNonPhi);
             nextNonPhi = nextNonPhi->getNextNode())
            ;
        IRBuilder builder(nextNonPhi);
        auto value = cast<Value>(I);
        auto type =
            FunctionType::get(builder.getVoidTy(), {value->getType()}, false);
        auto func = M.getOrInsertFunction("__spindle_record_value", type);
        builder.CreateCall(func, {value});
    }
};
