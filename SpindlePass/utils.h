#pragma once

#include "llvm/ADT/DenseMap.h"

using namespace llvm;

using ValueType = unsigned long long;
using SymbolTable = DenseMap<Value *, ValueType>;
