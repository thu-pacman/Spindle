#pragma once
// #define __DEBUG

#include <string>

//#include "../lib/IR/ConstantsContext.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

void preprocess(Module &M);

void check_nested_GEP(
    Instruction &I,
    std::vector<std::pair<Instruction *, Instruction *> > &replace_list);

template <typename T>
static std::string Print(T *value_or_type) {
    std::string str;
    llvm::raw_string_ostream stream(str);
    value_or_type->print(stream);
    return str;
}
