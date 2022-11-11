#include "utils.h"
#include <iostream>
#include <typeinfo>
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils.h"

// expand nested GEPInst
void preprocess(Module &M) {
    
    std::vector<std::pair<Instruction*, Instruction*> > replace_list;
    for (auto &F : M) {
        for (auto &BB : F) {
            for (auto &I : BB) {
                check_nested_GEP(I, replace_list);
            }
        }
    }
    assert(replace_list.size() == 0);
    // for (auto instrs : replace_list) {
    //     std::cout << "From: " << Print(instrs.first) << ", To: " << Print(instrs.second) << std::endl;
    //     ReplaceInstWithInst(instrs.first, instrs.second);
    // }
}

GetElementPtrInst* insert_GEPI_from_GEPO(GEPOperator* GEPO, Instruction& instr_before) {
    std::vector<Value*> operands_ref;
    for (auto use = GEPO->operands().begin() + 1; use != GEPO->operands().end();
        ++use) {
        operands_ref.push_back(use->get());
    }

    auto idx_arr = new ArrayRef<Value*>(operands_ref.data(), operands_ref.size());

    GetElementPtrInst* GEPI = nullptr;

    if (GEPO->isInBounds()) {
        GEPI = GetElementPtrInst::CreateInBounds(
                    GEPO->getPointerOperandType()->getPointerElementType(), 
                    GEPO->getPointerOperand(), 
                    *idx_arr,
                    "nested_GEP",
                    &instr_before
                );
    } else {
        GEPI = GetElementPtrInst::Create(
                    GEPO->getPointerOperandType()->getPointerElementType(), 
                    GEPO->getPointerOperand(), 
                    *idx_arr,
                    "nested_GEP",
                    &instr_before
                );
    }
    return GEPI;
}

void check_nested_GEP(Instruction& I, std::vector<std::pair<Instruction*, Instruction*> >& replace_list) {
    // two cases: 
    // 1. call
    // 2. load
    if (auto CallI = dyn_cast<CallInst>(&I)) {
        // CallI->getOperand
        bool has_nested_GEP = false;
        for (auto use = CallI->operands().begin(); use != CallI->operands().end(); ++use) {
            if (dyn_cast<GEPOperator>(use->get())) {
                has_nested_GEP = true;
            }
        }
        if (! has_nested_GEP) {
            return;
        }

        // std::vector<Value*> operands_ref;
        for (auto use = CallI->operands().begin(); use != CallI->operands().end(); ++use) {
            if (auto GEPO = dyn_cast<GEPOperator>(use->get())) {      
                GetElementPtrInst* GEPI = insert_GEPI_from_GEPO(GEPO, I);
                // operands_ref.push_back(GEPI);
                // replace operands
                use->set(GEPI);
            } 
        }

        // create a now instruction:
        // auto idx_arr = new ArrayRef<Value*>(operands_ref.data(), operands_ref.size());
        // auto func_callee = new FunctionCallee(CallI->getFunctionType(), CallI->getCalledFunction());
        // auto new_call_inst = CallInst::Create(
        //     *func_callee,
        //     *idx_arr, 
        //     "new_call_inst"
        // );
        // // auto new_call_inst = new CallInst(
        // //     CallI->getFunctionType(), 
        // //     CallI->getFunction(),
        // //     idx_arr, 
        // //     "new_call_inst", 
        // //     (Instruction*)nullptr
        // // );
        // replace_list.push_back(std::make_pair(&I, new_call_inst));
    }

    if (auto LoadI = dyn_cast<LoadInst>(&I)) {
        // if (auto GEPO = dyn_cast<GetElementPtrConstantExpr>(LoadI->getPointerOperand())) { // why i cannot do it ?
        if (auto GEPO = dyn_cast<GEPOperator>(LoadI->getPointerOperand())) { 
            auto GEPI = insert_GEPI_from_GEPO(GEPO, I); 
            // replace PointerOperand
            LoadI->setOperand(0, GEPI);

            // create a new instruction:
            // auto new_load_inst = new LoadInst(LoadI->getPointerOperandType()->getPointerElementType(), GEPI, "raw_Load", LoadI->isVolatile(), LoadI->getAlign());
            // replace_list.push_back(std::make_pair(&I, new_load_inst));
            // // std::cout << "converted GEPI: " << Print(GEPI) << std::endl;

        }
    }

}