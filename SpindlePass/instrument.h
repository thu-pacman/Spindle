#pragma once
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

class InstrumentationBase {
protected:
    Module &M;
    std::set<Value *> valueRecorded;
    SmallVector<GlobalValue *> globalRecorded;

public:
    explicit InstrumentationBase(Module &M) : M(M) {
    }

    [[nodiscard]] auto getName() const {
        return M.getName();
    }
    [[nodiscard]] auto &getInstrumentedSymbols() const {
        return valueRecorded;
    }
    [[nodiscard]] auto &getInstrumentedGlobals() const {
        return globalRecorded;
    }
    virtual void init_main(Instruction *I) const {
    }
    virtual void fini_main(Instruction *I) const {
    }
    virtual void record_br(BranchInst *I) const {
    }
    virtual void record_value(Value *V) {
        if (valueRecorded.find(V) == valueRecorded.end()) {
            valueRecorded.insert(V);
            if (auto GV = dyn_cast<GlobalValue>(V)) {
                globalRecorded.push_back(GV);
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
        IRBuilder<> *builder;
        if (auto I = dyn_cast<Instruction>(V)) {
            auto nextNonPhi = I->getNextNode();
            for (; nextNonPhi != nullptr && isa<PHINode>(nextNonPhi);
                 nextNonPhi = nextNonPhi->getNextNode())
                ;
            builder = new IRBuilder(nextNonPhi);
        } else {
            V = cast<GlobalValue>(V);
            builder =
                new IRBuilder(&M.getFunction("main")->getEntryBlock().front());
        }
        auto type =
            FunctionType::get(builder->getVoidTy(), {V->getType()}, false);
        auto func = M.getOrInsertFunction("__spindle_record_value", type);
        builder->CreateCall(func, {V});
        delete builder;
    }
};
