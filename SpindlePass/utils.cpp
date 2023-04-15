#include "utils.h"

#include <iostream>

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

// expand nested GEPInst
void preprocess(Module &M) {
    std::vector<std::pair<Instruction *, Instruction *>> replace_list;
    for (auto &F : M) {
        for (auto &BB : F) {
            for (auto &I : BB) {
                check_nested_GEP(I);
            }
        }
    }
    assert(replace_list.empty());
}

GetElementPtrInst *insert_GEPI_from_GEPO(GEPOperator *GEPO,
                                         Instruction &instr_before) {
    std::vector<Value *> operands_ref;
    for (auto use = GEPO->operands().begin() + 1; use != GEPO->operands().end();
         ++use) {
        operands_ref.push_back(use->get());
    }

    auto idx_arr =
        new ArrayRef<Value *>(operands_ref.data(), operands_ref.size());

    GetElementPtrInst *GEPI;

    if (GEPO->isInBounds()) {
        GEPI = GetElementPtrInst::CreateInBounds(
            GEPO->getPointerOperandType()->getPointerElementType(),
            GEPO->getPointerOperand(),
            *idx_arr,
            "nested_GEP",
            &instr_before);
    } else {
        GEPI = GetElementPtrInst::Create(
            GEPO->getPointerOperandType()->getPointerElementType(),
            GEPO->getPointerOperand(),
            *idx_arr,
            "nested_GEP",
            &instr_before);
    }
    return GEPI;
}

void check_nested_GEP(Instruction &I) {
    // two cases:
    // 1. call
    // 2. load
    if (auto CallI = dyn_cast<CallInst>(&I)) {
        // CallI->getOperand
        bool has_nested_GEP = false;
        for (auto &use : CallI->operands()) {
            if (dyn_cast<GEPOperator>(use.get())) {
                has_nested_GEP = true;
            }
        }
        if (!has_nested_GEP) {
            return;
        }

        // std::vector<Value*> operands_ref;
        for (auto &use : CallI->operands()) {
            if (auto GEPO = dyn_cast<GEPOperator>(use.get())) {
                GetElementPtrInst *GEPI = insert_GEPI_from_GEPO(GEPO, I);
                // replace operands
                use.set(GEPI);
            }
        }
    }

    if (auto LoadI = dyn_cast<LoadInst>(&I)) {
        if (auto GEPO = dyn_cast<GEPOperator>(LoadI->getPointerOperand())) {
            auto GEPI = insert_GEPI_from_GEPO(GEPO, I);
            // replace PointerOperand
            LoadI->setOperand(0, GEPI);
        }
    }
}