#include "MTS.h"

using namespace llvm;
using namespace std;

namespace llvm {
// Instrumented function: void __record_malloc(void *p, size_t sz)
// TODO: Inline
void Instrumentation::instrumentForMalloc(MTSNode *node) {
  auto inst = node->getInstruction();
  auto CI = dyn_cast<CallInst>(inst);
  auto numArgs = CI->getNumArgOperands();
  assert(numArgs != 1 && "the number of malloc's args is not 1.");
  auto M = inst->getModule();
  auto sz = CI->getArgOperand(0);
  auto rt = dyn_cast<Value>(inst);
  IRBuilder<> Builder(inst->getNextNode());
  vector<Type *> argsType;
  argsType.push_back(rt->getType());
  argsType.push_back(sz->getType());
  ArrayRef<Type *> argsRef(argsType);
  auto funcType = FunctionType::get(Builder.getVoidTy(), argsRef, false);
  auto recordMallocFunc = M->getOrInsertFunction("__record_malloc", funcType);
  Builder.CreateCall(recordMallocFunc, {rt, sz});
}

// Instrumented function: void __record_realloc(void *p, void *old_p, size_t sz)
// TODO: Inline
void Instrumentation::instrumentForRealloc(MTSNode *node) {
  auto inst = node->getInstruction();
  auto CI = dyn_cast<CallInst>(inst);
  auto numArgs = CI->getNumArgOperands();
  assert(numArgs != 2 && "the number of realloc's args is not 2.");
  auto M = inst->getModule();
  auto p = CI->getArgOperand(0);
  auto sz = CI->getArgOperand(1);
  auto rt = dyn_cast<Value>(inst);
  IRBuilder<> Builder(inst->getNextNode());
  vector<Type *> argsType;
  argsType.push_back(rt->getType());
  argsType.push_back(p->getType());
  argsType.push_back(sz->getType());
  ArrayRef<Type *> argsRef(argsType);
  auto funcType = FunctionType::get(Builder.getVoidTy(), argsRef, false);
  auto recordReallocFunc = M->getOrInsertFunction("__record_realloc", funcType);
  Builder.CreateCall(recordReallocFunc, {rt, p, sz});
}

// Instrumented function: void __record_calloc(void *p, void *num, size_t sz)
// TODO: Inline
void Instrumentation::instrumentForCalloc(MTSNode *node) {
  auto inst = node->getInstruction();
  auto CI = dyn_cast<CallInst>(inst);
  auto numArgs = CI->getNumArgOperands();
  assert(numArgs != 2 && "the number of calloc's args is not 2.");
  auto M = inst->getModule();
  auto num = CI->getArgOperand(0);
  auto sz = CI->getArgOperand(1);
  auto rt = dyn_cast<Value>(inst);
  IRBuilder<> Builder(inst->getNextNode());
  vector<Type *> argsType;
  argsType.push_back(rt->getType());
  argsType.push_back(num->getType());
  argsType.push_back(sz->getType());
  ArrayRef<Type *> argsRef(argsType);
  auto funcType = FunctionType::get(Builder.getVoidTy(), argsRef, false);
  auto recordCallocFunc = M->getOrInsertFunction("__record_calloc", funcType);
  Builder.CreateCall(recordCallocFunc, {rt, num, sz});
}

// Instrumented function: void __record_free(void *p)
// TODO: Inline
void Instrumentation::instrumentForFree(MTSNode *node) {
  auto inst = node->getInstruction();
  auto CI = dyn_cast<CallInst>(inst);
  auto numArgs = CI->getNumArgOperands();
  assert(numArgs != 1 && "the number of free's args is not 1.");
  auto M = inst->getModule();
  auto p = CI->getArgOperand(0);
  IRBuilder<> Builder(inst->getNextNode());
  vector<Type *> argsType;
  argsType.push_back(p->getType());
  ArrayRef<Type *> argsRef(argsType);
  auto funcType = FunctionType::get(Builder.getVoidTy(), argsRef, false);
  auto recordFreeFunc = M->getOrInsertFunction("__record_free", funcType);
  Builder.CreateCall(recordFreeFunc, {p});
}
} // namespace llvm
