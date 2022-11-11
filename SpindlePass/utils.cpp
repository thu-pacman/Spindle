#include "utils.h"
#include <iostream>
#include <typeinfo>

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
    for (auto instrs : replace_list) {
        std::cout << "From: " << Print(instrs.first) << ", To: " << Print(instrs.second) << std::endl;
        ReplaceInstWithInst(instrs.first, instrs.second);
    }
}

GetElementPtrInst* insert_GEPI_from_GEPO(GEPOperator* GEPO, Instruction& instr_before) {
    std::vector<Value*> operands_ref;
    for (auto use = GEPO->operands().begin() + 1; use != GEPO->operands().end();
        ++use) {
        operands_ref.push_back(use->get());
        // std::cout << "use->get(): " << Print(use->get()) << std::endl;
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
        // std::cout << "NumOperands: " << CallI->getNumOperands() << std::endl;
        // std::cout << "operands of CallI: " << std::endl;
        for (auto use = CallI->operands().begin(); use != CallI->operands().end(); ++use) {
            std::cout << " " << Print(use->get()) << std::endl;
            if (dyn_cast<GEPOperator>(use->get())) {
                has_nested_GEP = true;
                // break;
            }
        }
        std::cout << std::endl;

        if (! has_nested_GEP) {
            return;
        }

        // inline CallInst(FunctionType *Ty, Value *Func, ArrayRef<Value *> Args,
        //           const Twine &NameStr, Instruction *InsertBefore)
        // static CallInst *Create(FunctionType *Ty, Value *Func, ArrayRef<Value *> Args,
        //                   const Twine &NameStr,
        //                   Instruction *InsertBefore = nullptr)

        // std::vector<Value*> operands_ref;
        std::cout << "CallInst's operands !!!" << std::endl;
        for (auto use = CallI->operands().begin(); use != CallI->operands().end(); ++use) {
            std::cout << "use->get(): " << Print(use->get()) << std::endl;
            if (auto GEPO = dyn_cast<GEPOperator>(use->get())) {      
                GetElementPtrInst* GEPI = insert_GEPI_from_GEPO(GEPO, I);
                // operands_ref.push_back(GEPI);
                // replace operands
                use->set(GEPI);
            } 
        }

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

    // return;

    if (auto LoadI = dyn_cast<LoadInst>(&I)) {
        // std::cout << "\ntype of LoadInst: " << typeid(*LoadI).name() << std::endl;
        // auto GEPI = dyn_cast<Value>(LoadI->getOperand(0));
        // std::cout << "GEPInst nested in LoadInst: " << Print(GEPI) << std::endl;
        // std::cout << "type of it: " << typeid(*GEPI).name() << std::endl;
        // std::cout << "type of dyn_cast<Value>: " << typeid(*dyn_cast<Value>(&I)).name() << std::endl;
        
        // if (auto GEPO = dyn_cast<GetElementPtrConstantExpr>(LoadI->getPointerOperand())) { // why i cannot do it ?
        if (auto GEPO = dyn_cast<GEPOperator>(LoadI->getPointerOperand())) {
            std::cout << "is GEPOperator: " << Print(GEPO) << std::endl;

            
            std::vector<Value*> operands_ref;
            for (auto use = GEPO->operands().begin() + 1; use != GEPO->operands().end();
                ++use) {
                operands_ref.push_back(use->get());
                std::cout << "use->get(): " << Print(use->get()) << std::endl;
            }

            // auto pointer_operand = GEPO->getPointerOperand();
            // auto type_ = cast<PointerType>(pointer_operand->getType()->getScalarType());
            // // std::cout << "pointer_operand's type: " << Print(type_) << std::endl;
            // auto got_type_ = GEPO->getPointerOperandType();
            // // std::cout << "got_type_: " << Print(got_type_) << std::endl;
            // assert(type_->isOpaqueOrPointeeTypeMatches(got_type_->getPointerElementType()));

            auto idx_arr = new ArrayRef<Value*>(operands_ref.data(), operands_ref.size());

            GetElementPtrInst* GEPI = nullptr;

            if (GEPO->isInBounds()) {
                GEPI = GetElementPtrInst::CreateInBounds(
                            GEPO->getPointerOperandType()->getPointerElementType(), 
                            GEPO->getPointerOperand(), 
                            *idx_arr,
                            "nested_GEP",
                            &I
                        );
            } else {
                GEPI = GetElementPtrInst::Create(
                            GEPO->getPointerOperandType()->getPointerElementType(), 
                            GEPO->getPointerOperand(), 
                            *idx_arr,
                            "nested_GEP",
                            &I
                        );
            }
            // replace PointerOperand
            LoadI->setOperand(0, GEPI);

            // auto new_load_inst = new LoadInst(LoadI->getPointerOperandType()->getPointerElementType(), GEPI, "raw_Load", LoadI->isVolatile(), LoadI->getAlign());
            // replace_list.push_back(std::make_pair(&I, new_load_inst));
            // // std::cout << "converted GEPI: " << Print(GEPI) << std::endl;

        } else {
            std::cout << "no GEPOperator nested !!!" << std::endl;
        }
    }

}