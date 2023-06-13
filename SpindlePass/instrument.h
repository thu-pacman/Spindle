#pragma once
#include "llvm/IR/IRBuilder.h"

using namespace llvm;
class InstrumentationBase {
protected:
    Module &M;
    std::set<Value *> valueRecorded;

public:
    explicit InstrumentationBase(Module &M) : M(M) {
    }
    [[nodiscard]] auto getName() const {
        return M.getName();
    }
    virtual void init_main(Instruction *I) const {
    }
    virtual void fini_main(Instruction *I) const {
    }
    virtual void record_br(BranchInst *I) const {
    }
    virtual void record_value(Value *V) {
    }
};

class InstrumentationDummy : public InstrumentationBase {
    StringMap<SmallVector<GlobalValue *>> funcArgRecorded;

public:
    explicit InstrumentationDummy(Module &M) : InstrumentationBase(M) {
    }
    [[nodiscard]] auto &getInstrumentedSymbols() const {
        return valueRecorded;
    }
    [[nodiscard]] auto &getInstrumentedArgs(StringRef funcName) {
        return funcArgRecorded[funcName];
    }
    void record_value(Value *V) override {
        if (valueRecorded.find(V) == valueRecorded.end()) {
            valueRecorded.insert(V);
            if (auto GV = dyn_cast<GlobalValue>(V)) {
                funcArgRecorded["main"].push_back(GV);
            }
        }
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
        IRBuilder builder(I);
        auto type =
            FunctionType::get(builder.getVoidTy(),
                              {I->getCondition()->getType()},
                              false);  // isVarArg: indicate that the number of
                                       // arguments are variable
        auto func = M.getOrInsertFunction("__spindle_record_br", type);
        builder.CreateCall(func, {I->getCondition()});
    }
    void record_value(Value *V) override {
        if (valueRecorded.count(V)) {
            return;
        }
        valueRecorded.insert(V);
        if (auto I = dyn_cast<Instruction>(V)) {
            record_value(V, I->getNextNode());
        } else {
            auto funcName = isa<GlobalValue>(V)
                                ? "main"
                                : cast<Argument>(V)->getParent()->getName();
            record_value(V, &M.getFunction(funcName)->getEntryBlock().front());
        }
    }
    void record_value(Value *V, Instruction *I) {
        for (; isa<PHINode>(I); I = I->getNextNode())
            ;
        IRBuilder builder(I);
        auto type =
            FunctionType::get(builder.getVoidTy(), {V->getType()}, false);
        auto func = M.getOrInsertFunction("__spindle_record_value", type);
        builder.CreateCall(func, {V});
    }
};
