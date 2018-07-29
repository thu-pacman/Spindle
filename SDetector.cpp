#include "SDetector.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;
using namespace std;

namespace llvm {

static int idx = 0;

static bool isMonotonicPattern(string &pat) {
  // return false;
  if (count(pat.begin(), pat.end(), 'l') + count(pat.begin(), pat.end(), 's') ==
          1 &&
      count(pat.begin(), pat.end(), 'p') <= 1)
    return true;
  return false;
}

static bool isGlobalAccess(string &pat) {
  if (pat[1] != 'g')
    return false;
  for (size_t i = 2, sz = pat.size(); i < sz; ++i) {
    if (!isdigit(pat[i]))
      return false;
  }
  return true;
}

static bool instrumentBeforeInstruction(Instruction *inst, StringRef &fname,
                                        Type *retTy, ArrayRef<Value *> &args) {
  auto M = inst->getModule();
  if (PHINode::classof(inst)) {
    errs() << "Cannot insert function before PHINode.\n";
    return false;
  }
  IRBuilder<> Builder(inst);
  vector<Type *> argsType;
  for (auto arg : args) {
    argsType.push_back(arg->getType());
  }
  ArrayRef<Type *> argsRef(argsType);
  auto funcType = FunctionType::get(retTy, argsRef, false);
  auto func = M->getOrInsertFunction(fname, funcType);
  Builder.CreateCall(func, args);
  return true;
}

static bool instrumentAfterInstruction(Instruction *inst, StringRef &fname,
                                       Type *retTy, ArrayRef<Value *> &args) {
  if (TerminatorInst::classof(inst)) {
    errs() << "Cannnot insert function after terminator inst\n";
    return false;
  }
  return instrumentBeforeInstruction(inst->getNextNode(), fname, retTy, args);
}

static bool instrumentAtBeginOfBasicBlock(BasicBlock *bb, StringRef &fname,
                                          Type *retTy,
                                          ArrayRef<Value *> &args) {
  auto inst = &bb->front();
  while (PHINode::classof(inst)) {
    inst = inst->getNextNode();
  }
  return instrumentBeforeInstruction(inst, fname, retTy, args);
}

static bool instrumentAtEndOfBasicBlock(BasicBlock *bb, StringRef &fname,
                                        Type *retTy, ArrayRef<Value *> &args) {
  Instruction *inst = bb->getTerminator();
  return instrumentBeforeInstruction(inst, fname, retTy, args);
}

static bool instrumentAtBeginOfFunction(Function *f, StringRef &fname,
                                        Type *retTy, ArrayRef<Value *> &args) {
  Instruction *inst = &f->front().front();
  return instrumentBeforeInstruction(inst, fname, retTy, args);
}

void SDetector::runDetector() {
  // errs() << "Running detector\n";
  // collectBasementArg();
  for (auto MF : MMC) {
    for (auto BB : *MF) {
      for (auto node : *BB) {
        switch (node->getKind()) {
        case MTSNode::CallKind: {
          auto CI = dyn_cast<CallInst>(node->getInstruction());
          auto fn = CI->getCalledFunction();
          // TODO: Function pointer, virtual function.
          if (!fn)
            continue;
          auto fname = fn->getName();
          if (fname.equals("malloc"))
            node->addInstrumentation(instrumentForMalloc);
          else if (fname.equals("realloc"))
            node->addInstrumentation(instrumentForRealloc);
          else if (fname.equals("calloc"))
            node->addInstrumentation(instrumentForCalloc);
          else if (fname.equals("free"))
            node->addInstrumentation(instrumentForFree);
        } break;
        case MTSNode::LoadKind:
        case MTSNode::StoreKind:
          analysisMemAccess(dyn_cast<MTSMemAccess>(node));
          break;
        case MTSNode::AllocaKind: {
          // StringRef fname = "__is_in_range";
          // vector<Value *> args;
          // // args.push_back(base);
          // // args.push_back(mn->getInstruction());
          // ArrayRef<Value *> argsRef(args);
          // auto M = node->getInstruction()->getModule();
          // auto voidTy = Type::getVoidTy(M->getContext());
          // instrumentAfterInstruction(node->getInstruction(), fname, voidTy,
          //                            argsRef);
        } break;
        default:
          break;
        }
      }
    }
  }
}

void SDetector::collectBasementArg() {
  for (auto MF : MMC) {
    auto attr = new SDFunctionAttr();
    MF->setAttr(attr);
    for (auto MBB : *MF) {
      for (auto node : *MBB) {
      }
    }
  }
}

void SDetector::analysisLoad(MTSLoad *ln) {}

void SDetector::analysisStore(MTSStore *sn) {}
// Instrumented function: void __record_malloc(void *p, size_t sz)
// TODO: Inline
void SDetector::instrumentForMalloc(MTSNode *node) {
  // errs() << "Instrumenting for malloc\n";
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
void SDetector::instrumentForRealloc(MTSNode *node) {
  // errs() << "Instrumenting for realloc\n";
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
void SDetector::instrumentForCalloc(MTSNode *node) {
  // errs() << "Instrumenting for calloc\n";
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
void SDetector::instrumentForFree(MTSNode *node) {
  // errs() << "Instrumenting for free\n";
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

void SDetector::analysisMemAccess(MTSMemAccess *mn) {
  auto M = mn->getInstruction()->getModule();
  auto voidTy = Type::getVoidTy(M->getContext());
  auto base = mn->getBasement();
  errs() << "Base: ";
  base->print(errs());
  errs() << "\n";

  errs() << "inst: ";
  mn->getInstruction()->print(errs());
  errs() << "\n";
  base->print(errs());
  errs() << "base\n";
  // Constant, don't have to instrument.
  if (mn->mustTakeFullInstrumentation()) {
    // if (true) {
    StringRef fname = "__is_in_range";
    vector<Value *> args;
    args.push_back(base);
    // errs() << "Full instrumentation\n";
    Value *ptr;
    if (MTSLoad::classof(mn)) {
      ptr = dyn_cast<LoadInst>(mn->getInstruction())->getPointerOperand();
      errs() << "load inst ptr: ";
    } else {
      ptr = dyn_cast<StoreInst>(mn->getInstruction())->getPointerOperand();
      errs() << "store inst ptr: ";
    }
    ptr->print(errs());
    args.push_back(ptr);
    errs() << "\n";
    ArrayRef<Value *> argsRef(args);
    // args.push_back(ConstantInt::get(Type::getInt32Ty(M->getContext()),
    // idx)); errs() << "badref " << idx << ": ";
    // mn->getInstruction()->print(errs());
    // errs() << "\n";
    // ++idx;
    // instrumentAfterInstruction(mn->getInstruction(), fname, voidTy,
    // argsRef);
    instrumentAtEndOfBasicBlock(mn->getInstruction()->getParent(), fname,
                                voidTy, argsRef);
    return;
  }
  if (mn->isDependenciesConstant()) {
    return;
  }
  auto deps = mn->getDependencies();
  if (deps.size() == 1) {
    // errs() << "loop\n";
    // Only dependent on 1 loop indvar.
    if (auto ml = mn->tryToGetMyLoopFor(deps[0])) {
      auto exitBlocks = ml->getExitBlocks();
      // TODO: Add instrumentation.
      StringRef fname = "__is_in_range_at_loop_end";
      vector<Value *> args;
      args.push_back(base);
      // args.push_back(mn->getInstruction());
      Value *ptr;
      if (MTSLoad::classof(mn)) {
        ptr = dyn_cast<LoadInst>(mn->getInstruction())->getPointerOperand();
        errs() << "load inst ptr: ";
      } else {
        ptr = dyn_cast<StoreInst>(mn->getInstruction())->getPointerOperand();
        errs() << "store inst ptr: ";
      }
      ptr->print(errs());
      args.push_back(ptr);
      errs() << "\n";
      // args.push_back(ConstantInt::get(Type::getInt32Ty(M->getContext()),
      // idx)); errs() << "badref " << idx << ": ";
      // mn->getInstruction()->print(errs());
      // errs() << "\n";
      // ++idx;
      ArrayRef<Value *> argsRef(args);
      for (auto exitBlock : exitBlocks) {
        instrumentAtEndOfBasicBlock(exitBlock, fname, voidTy, argsRef);
      }
    }
    // Only have 1 dependency.
    else {

      // errs() << "1dep\n";
      auto dep = deps[0];
      StringRef fname = "__is_in_range";
      vector<Value *> args;
      args.push_back(base);
      errs() << "Full instrumentation\n";
      Value *ptr;
      if (MTSLoad::classof(mn)) {
        ptr = dyn_cast<LoadInst>(mn->getInstruction())->getPointerOperand();
        errs() << "load inst ptr: ";
      } else {
        ptr = dyn_cast<StoreInst>(mn->getInstruction())->getPointerOperand();
        errs() << "store inst ptr: ";
      }
      ptr->print(errs());
      args.push_back(ptr);
      errs() << "\n";
      // errs() << "badref " << idx << ": ";
      // mn->getInstruction()->print(errs());
      // errs() << "\n";
      // ++idx;
      ArrayRef<Value *> argsRef(args);
      if (Instruction::classof(base) && Instruction::classof(dep)) {
        // Instruction *inst;
        // DominatorTree DT =
        //     DominatorTree(mn->getParent().getParent().getFunction());
        // auto baseinst = dyn_cast<Instruction>(base),
        //      depinst = dyn_cast<Instruction>(dep);
        // if (DT.dominates(baseinst, depinst)) {
        //   inst = depinst;
        // } else if (DT.dominates(depinst, baseinst)) {
        //   inst = depinst;
        // } else {
        //   errs() << "basement and dependency don't dominate\n";
        //   exit(1);
        // }
        // instrumentAfterInstruction(inst, fname, voidTy, argsRef);
        // instrumentAfterInstruction(mn->getInstruction(), fname, voidTy,
        //                            argsRef);
        instrumentAtEndOfBasicBlock(mn->getInstruction()->getParent(), fname,
                                    voidTy, argsRef);
      } else if (Instruction::classof(base)) {
        // instrumentAfterInstruction(dyn_cast<Instruction>(base), fname,
        // voidTy,
        //                            argsRef);
        // instrumentAfterInstruction(mn->getInstruction(), fname, voidTy,
        //                            argsRef);
        instrumentAtEndOfBasicBlock(mn->getInstruction()->getParent(), fname,
                                    voidTy, argsRef);
      } else if (Instruction::classof(dep)) {
        // instrumentAfterInstruction(dyn_cast<Instruction>(dep), fname,
        // voidTy,
        //                            argsRef);
        // instrumentAfterInstruction(mn->getInstruction(), fname, voidTy,
        //                            argsRef);
        instrumentAtEndOfBasicBlock(mn->getInstruction()->getParent(), fname,
                                    voidTy, argsRef);
      } else {
        // instrumentAtBeginOfFunction(&mn->getParent().getParent().getFunction(),
        //                             fname, voidTy, argsRef);
        instrumentAtEndOfBasicBlock(mn->getInstruction()->getParent(), fname,
                                    voidTy, argsRef);
      }
    }
  } else {
    bool allIndvar = true;
    for (auto dep : deps) {
      if (!mn->isDependencyLoopIndvar(dep)) {
        allIndvar = false;
        break;
      }
    }
    // All the dependencies are loop indvar.
    if (allIndvar) {
      // errs() << "multiloop\n";
      auto outest = min_element(
          deps.begin(), deps.end(), [&, mn](Value *lhs, Value *rhs) {
            auto ld = mn->tryToGetMyLoopFor(lhs)->getLoopDepth();
            auto rd = mn->tryToGetMyLoopFor(rhs)->getLoopDepth();
            return ld < rd;
          });
      auto ml = mn->tryToGetMyLoopFor(*outest);
      auto exitBlocks = ml->getExitBlocks();
      // TODO: Add instrumentation.
      StringRef fname = "__is_in_range_at_multi_loop_end";
      vector<Value *> args;
      args.push_back(base);
      Value *ptr;
      if (MTSLoad::classof(mn)) {
        ptr = dyn_cast<LoadInst>(mn->getInstruction())->getPointerOperand();
        errs() << "load inst ptr: ";
      } else {
        ptr = dyn_cast<StoreInst>(mn->getInstruction())->getPointerOperand();
        errs() << "store inst ptr: ";
      }
      ptr->print(errs());
      args.push_back(ptr);
      errs() << "\n";
      // args.push_back(mn->getInstruction());
      // args.push_back(ConstantInt::get(Type::getInt32Ty(M->getContext()),
      // idx));
      // errs() << "badref " << idx << ": ";
      // mn->getInstruction()->print(errs());
      // errs() << "\n";
      ++idx;
      if (mn->getInstruction() == nullptr)
        return;
      ArrayRef<Value *> argsRef(args);
      for (auto exitBlock : exitBlocks) {
        // instrumentAtEndOfBasicBlock(exitBlock, fname, voidTy, argsRef);
        instrumentAtBeginOfBasicBlock(exitBlock, fname, voidTy, argsRef);
      }
    }
    // Not all the dependencies are loop indvar.
    // Simply instrument.
    else {
      // errs() << "multidep\n";
      StringRef fname = "__is_in_range";
      vector<Value *> args;
      args.push_back(base);
      Value *ptr;
      if (MTSLoad::classof(mn)) {
        ptr = dyn_cast<LoadInst>(mn->getInstruction())->getPointerOperand();
        errs() << "load inst ptr: ";
      } else {
        ptr = dyn_cast<StoreInst>(mn->getInstruction())->getPointerOperand();
        errs() << "store inst ptr: ";
      }
      ptr->print(errs());
      args.push_back(ptr);
      errs() << "\n";
      errs() << "Full instrumentation\n";
      // args.push_back(mn->getInstruction());
      ArrayRef<Value *> argsRef(args);
      instrumentAfterInstruction(mn->getInstruction(), fname, voidTy, argsRef);
    }
  }
} // namespace llvm

} // namespace llvm
