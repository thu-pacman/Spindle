#ifndef LLVM_MTS_H
#define LLVM_MTS_H

#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Instructions.h"
#include <set>
#include <vector>

using namespace llvm;
using namespace std;

namespace llvm {

void printDebugLocIfPosibble(Value *v);

class MyFunction;
class MyBasicBlock;
class MyLoop;
class LoopIndvar;
class MTSNode;

// class AttrBase
// ========================================================

class AttrBase {};

// class Instrumentation
// =========================================================

class Instrumentation {
public:
  using InstrumentationFuncType = function<void(MTSNode *)>;

private:
  InstrumentationFuncType InstrumentFunction;

public:
  Instrumentation(InstrumentationFuncType func) : InstrumentFunction(func) {}

  void doInstrumentation(MTSNode *node) { InstrumentFunction(node); }
};

// class Pattern
// ===========================================================

class Pattern {
  string pat;

  size_t cursor;

public:
  Pattern(string str) : pat(str), cursor(0) {}

  Pattern() : pat("") {}

  string &str() { return pat; }

  // If a pattern is constant expression.
  // A constant pattern should only contains numbers, arithmetic operators and
  // the constant prefix 'c'.
  // TODO: More rules may be included here.
  static bool isConstantPattern(string &str) {
    for (auto c : str) {
      if (isalpha(c) && c != 'c')
        return false;
    }
    return true;
  }

  void clear() { cursor = 0; }

  pair<char, unsigned> next() {
    auto sz = pat.size();
    if (cursor == sz) {
      clear();
      return {};
    }
    char ty = pat[cursor++];
    unsigned id = 0;
    while (cursor < sz && isdigit(pat[cursor])) {
      id = id * 10 + pat[cursor++] - '0';
    }
    return {ty, id};
  }
};

// class MTSNode and subclasses
// ==========================================================

// Record the ID of MTSNode.
static unsigned MTSID = 0;

class MTSNode {
public:
  enum MTSKind : unsigned {
    // For the instructions that not included in this enum.
    // May be added in the future.
    UnhandledKind,
    BinaryOpKind,
    CallKind,
    CmpKind,
    GetElementPtrKind,
    LandingPadKind,
    PhiNodeKind,
    SelectKind,
    StoreKind,
    // For TerminatorInst
    ReturnKind,
    SwitchKind,
    BranchKind,
    // For UnaryInstruction
    AllocaKind,
    CastKind,
    LoadKind,
  };

private:
  MTSKind Kind;

  Value &TheValue;

  MyBasicBlock &Parent;

  Pattern Pat;

  const unsigned ID;

  vector<Instrumentation *> Instrumentations;

  AttrBase *Attr;

protected:
  MTSNode(Value &V, MTSKind K, MyBasicBlock &Parent)
      : TheValue(V), Kind(K), Parent(Parent), ID(MTSID++), Attr(nullptr) {}

  // For derived class setting Pattern.
  // TODO: Too slow!
  void setPattern(string p) { Pat = p; }

public:
  string &getPatternString() { return Pat.str(); }

  Pattern &getPattern() { return Pat; }

  Instruction *getInstruction() { return dyn_cast<Instruction>(&TheValue); }

  MyBasicBlock &getParent() { return Parent; }

  virtual void computePattern(vector<string> &args) = 0;

  MTSKind getKind() const { return Kind; }

  const unsigned getID() const { return ID; }

  void addInstrumentation(Instrumentation::InstrumentationFuncType func) {
    Instrumentations.push_back(new Instrumentation(func));
  }

  vector<Instrumentation *> &getInstrumentations() { return Instrumentations; }

  void setAttr(AttrBase *p) { Attr = p; }

  AttrBase *getAttr() { return Attr; }
};

// Unhandled kind of MTSNode, default
class MTSUnhandled : public MTSNode {
public:
  MTSUnhandled(Value &V, MyBasicBlock &Parent)
      : MTSNode(V, UnhandledKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == UnhandledKind;
  }
};

// BinaryOperator inst
class MTSBinaryOp : public MTSNode {
public:
  MTSBinaryOp(BinaryOperator &BOI, MyBasicBlock &Parent)
      : MTSNode(BOI, BinaryOpKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == BinaryOpKind;
  }
};

// Call inst
class MTSCall : public MTSNode {
public:
  MTSCall(CallInst &CI, MyBasicBlock &Parent) : MTSNode(CI, CallKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == CallKind;
  }
};

// Cmp inst
class MTSCmp : public MTSNode {
public:
  MTSCmp(CmpInst &CI, MyBasicBlock &Parent) : MTSNode(CI, CmpKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == CmpKind;
  }
};

// GetElementPtr inst
class MTSGetElementPtr : public MTSNode {
public:
  MTSGetElementPtr(GetElementPtrInst &GEP, MyBasicBlock &Parent)
      : MTSNode(GEP, GetElementPtrKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == GetElementPtrKind;
  }
};

// LandkingPad inst
class MTSLandingPad : public MTSNode {
public:
  MTSLandingPad(LandingPadInst &LP, MyBasicBlock &Parent)
      : MTSNode(LP, LandingPadKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == LandingPadKind;
  }
};

// PhiNode inst
class MTSPhiNode : public MTSNode {
  LoopIndvar *IndvarOf;

public:
  MTSPhiNode(PHINode &PHI, MyBasicBlock &Parent)
      : MTSNode(PHI, PhiNodeKind, Parent), IndvarOf(nullptr) {}

  void computePattern(vector<string> &args);

  LoopIndvar *getIndvarOf() { return IndvarOf; }

  MyLoop *tryToGetMyLoop();
  void setIndvarOf(LoopIndvar *LI) { IndvarOf = LI; }

  bool isLoopIndvar() { return IndvarOf != nullptr; }

  static bool classof(const MTSNode *node) {
    return node->getKind() == PhiNodeKind;
  }
};

// Select inst
class MTSSelect : public MTSNode {
public:
  MTSSelect(SelectInst &SI, MyBasicBlock &Parent)
      : MTSNode(SI, SelectKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == SelectKind;
  }
};

// Return inst
class MTSReturn : public MTSNode {
public:
  MTSReturn(ReturnInst &RI, MyBasicBlock &Parent)
      : MTSNode(RI, ReturnKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == ReturnKind;
  }
};

// Switch inst
class MTSSwitch : public MTSNode {
public:
  MTSSwitch(SwitchInst &SI, MyBasicBlock &Parent)
      : MTSNode(SI, SwitchKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == SwitchKind;
  }
};

// Branch inst
class MTSBranch : public MTSNode {
public:
  MTSBranch(BranchInst &BI, MyBasicBlock &Parent)
      : MTSNode(BI, BranchKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == BranchKind;
  }
};

// Alloca inst
class MTSAlloca : public MTSNode {
public:
  MTSAlloca(AllocaInst &AI, MyBasicBlock &Parent)
      : MTSNode(AI, AllocaKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == AllocaKind;
  }
};

// Cast inst
class MTSCast : public MTSNode {
public:
  MTSCast(CastInst &CI, MyBasicBlock &Parent) : MTSNode(CI, CastKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == CastKind;
  }
};

class MTSMemAccess : public MTSNode {
protected:
  Value *Basement;

  vector<Value *> Dependencies;

  // bool IsConstant;

  bool FullInstrumentation;

  void analysisMemAccess(Value *addr);

public:
  MTSMemAccess(Value &V, MTSKind K, MyBasicBlock &Parent)
      : MTSNode(V, K, Parent), FullInstrumentation(false) {}

  static bool classof(const MTSNode *node) {
    return node->getKind() == StoreKind || node->getKind() == LoadKind;
  }

  bool isDependenciesConstant() { return Dependencies.empty(); }

  vector<Value *> &getDependencies() { return Dependencies; }

  Value *getBasement() { return Basement; }

  bool isDependencyLoopIndvar(Value *v);

  MyLoop *tryToGetMyLoopFor(Value *v);

  bool mustTakeFullInstrumentation() { return FullInstrumentation; }
};

// Store inst
class MTSStore : public MTSMemAccess {
public:
  MTSStore(StoreInst &SI, MyBasicBlock &Parent)
      : MTSMemAccess(SI, StoreKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == StoreKind;
  }
};

// Load inst
class MTSLoad : public MTSMemAccess {
public:
  MTSLoad(LoadInst &LI, MyBasicBlock &Parent)
      : MTSMemAccess(LI, LoadKind, Parent) {}

  void computePattern(vector<string> &args);

  static bool classof(const MTSNode *node) {
    return node->getKind() == LoadKind;
  }
};

// class LoopIndvar.
// Record one of the induction variables for a specific Loop.
// A loop may contain one or more loop induction variables.
class LoopIndvar {
  MTSPhiNode &PHI;

  MyLoop *ML;

  Value *InitValue;

  Value *FiniValue;

  string Step;

  char OpSymbol;

  int Incresement;

public:
  LoopIndvar(MTSPhiNode &PHI) : PHI(PHI) { PHI.setIndvarOf(this); }

  LoopIndvar(MTSPhiNode &PHI, MyLoop *ML, char OpSymbol, int Incresement)
      : PHI(PHI), ML(ML), OpSymbol(OpSymbol), Incresement(Incresement) {
    // errs() << "set phi as indvar\n";
    PHI.setIndvarOf(this);
  }

  MyLoop *getMyLoop() { return ML; }
};

// class MyLoop.
class MyLoop {
  Loop &TheLoop;

  MyFunction &Parent;

  int Depth;

  const unsigned ID;

  vector<LoopIndvar *> Indvars;

  bool Analyzed;

  BasicBlock *preheader;

  BasicBlock *latch;

  BasicBlock *header;

  SmallVector<BasicBlock *, 8> exitBlocks;

  SmallVector<BasicBlock *, 8> exitingBlocks;

  BasicBlock *exitBlock;

  BasicBlock *exitingBlock;

  AttrBase *Attr;

public:
  MyLoop(Loop &L, unsigned id, MyFunction &Parent)
      : TheLoop(L), ID(id), Parent(Parent), Analyzed(false), Attr(nullptr) {
    preheader = L.getLoopPreheader();
    latch = L.getLoopLatch();
    header = L.getHeader();
    L.getExitBlocks(exitBlocks);
    L.getExitingBlocks(exitingBlocks);
    exitBlock = L.getExitBlock();
    exitingBlock = L.getExitingBlock();
    Depth = L.getLoopDepth();
  }

  Loop &getLoop() { return TheLoop; }

  vector<LoopIndvar *> getIndvars() { return Indvars; }

  const unsigned getID() { return ID; }

  void computeIndvars();

  BasicBlock *getLoopPreheader() { return preheader; }

  BasicBlock *getLoopLatch() { return latch; }

  BasicBlock *getHeader() { return header; }

  SmallVector<BasicBlock *, 8> &getExitBlocks() { return exitBlocks; }

  SmallVector<BasicBlock *, 8> &getExitingBlocks() { return exitingBlocks; }

  BasicBlock *getExitBlock() { return exitBlock; }

  BasicBlock *getExitingBlock() { return exitingBlock; }

  void setAttr(AttrBase *p) { Attr = p; }

  AttrBase *getAttr() { return Attr; }

  int getLoopDepth() { return Depth; }
};

// class MyModuleContext
// Class that stores the Module context including GlobalVariables, DataLayout,
// ...
// TODO: Add all the functions to MyModule class so that the inter-procedual
// analysis can be more OO.
// TODO: If there are multiple modules, how to manage the ID for the lists like
// GlobalVariable and Function?
class MyModuleContext {
  Module &TheModule;

  // GlobalVariable pointer to global id.
  map<GlobalVariable *, unsigned> GVars;

  // vector<Function *> Funcs;

  // For function id fast finding. Function pointer to function id.
  map<Function *, unsigned> FuncsMap;

  vector<MyFunction *> FunctionList;

  DataLayout *DL;

  map<Value *, unsigned> UnhandledValues;

  AttrBase *Attr;

public:
  MyModuleContext(Module &M) : TheModule(M), Attr(nullptr) {
    unsigned idx = 0;
    for (auto &G : M.getGlobalList())
      GVars.insert({&G, idx++});

    DL = new DataLayout(&M);

    // TODO: Add lib function into this list.
    // idx = 0;
    // for (auto &it : TheModule)
    //   // Funcs.push_back(&it);
    //   FuncsMap.insert({&it, idx++});
  }

  map<GlobalVariable *, unsigned> &getGlobalMap() { return GVars; }

  // vector<Function *> &getFunctionList() { return Funcs; }

  map<Function *, unsigned> &getFunctionMap() { return FuncsMap; }

  DataLayout *getDataLayout() { return DL; }

  unsigned getOrInsertUnhandled(Value *v) {
    auto it = UnhandledValues.find(v);
    unsigned idx;
    if (it == UnhandledValues.end()) {
      idx = UnhandledValues.size();
      UnhandledValues.insert({v, idx});
    } else
      idx = it->second;
    return idx;
  }

  string getModuleName() { return TheModule.getName().str(); }

  Module &getModule() { return TheModule; }

  void addFunction(MyFunction *MF);

  void setAttr(AttrBase *p) { Attr = p; }

  AttrBase *getAttr() { return Attr; }

  vector<MyFunction *> &getFunctionList() { return FunctionList; }

  vector<MyFunction *>::iterator begin() { return FunctionList.begin(); }

  vector<MyFunction *>::iterator end() { return FunctionList.end(); }
};

// class MyBasicBlock.

// Record the ID of MyBasicBlock.
static unsigned MBBID = 0;
class MyBasicBlock {
  BasicBlock &TheBB;

  vector<MTSNode *> MTS;

  int LoopID;

  MyFunction &Parent;

  const unsigned ID;

  AttrBase *Attr;

  bool MemChange;

public:
  MyBasicBlock(BasicBlock &BB, MyFunction &MF)
      : TheBB(BB), LoopID(-1), Parent(MF), ID(MBBID++), Attr(nullptr),
        MemChange(false) {
    buildMTS();
    for (auto node : MTS) {
      if (auto ci = dyn_cast<CallInst>(node->getInstruction())) {
        if (auto fn = ci->getCalledFunction()) {
          auto fname = fn->getName();
          if (fname.equals("malloc") || fname.equals("realloc") ||
              fname.equals("calloc") || fname.equals("free"))
            MemChange = true;
        }
      }
    }
  }

  BasicBlock &getBB() { return TheBB; }

  void setLoopID(int id) { LoopID = id; }

  int getLoopID() { return LoopID; }

  void buildMTS();

  MyFunction &getParent() { return Parent; }

  vector<MTSNode *>::iterator begin() { return MTS.begin(); }

  vector<MTSNode *>::iterator end() { return MTS.end(); }

  const unsigned getID() { return ID; }

  void setAttr(AttrBase *p) { Attr = p; }

  AttrBase *getAttr() { return Attr; }

  bool isMemChange() { return MemChange; }

  friend class MyFunction;
};

// class MyFunction.
class MyFunction {
  Function &TheFunction;

  MyModuleContext &MMC;

  vector<MyBasicBlock *> BBs;

  vector<MyLoop *> Loops;

  map<Value *, MTSNode *> V2MTS;

  vector<Argument *> Args;

  void dependenceAnalysis();

  void getDependency(MTSNode *node);

  const unsigned ID;

  AttrBase *Attr;

  bool MemChange;

public:
  MyFunction(Function &F, MyModuleContext &MMC)
      : TheFunction(F), MMC(MMC), ID(MMC.getFunctionList().size()),
        Attr(nullptr), MemChange(false) {
    MMC.addFunction(this);
    for (auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
      Args.push_back(arg);
    }
    unsigned bbid = 0;
    for (auto &BB : F) {
      BBs.push_back(new MyBasicBlock(BB, *this));
    }
    for (auto MBB : BBs) {
      if (MBB->isMemChange()) {
        MemChange = true;
        break;
      }
    }
    dependenceAnalysis();
  }

  vector<MyBasicBlock *> &getBBs() { return BBs; }

  MyLoop *addLoop(Loop &L) {
    Loops.push_back(new MyLoop(L, Loops.size(), *this));
    auto ML = Loops.back();
    return ML;
  }

  vector<MyBasicBlock *>::iterator begin() { return BBs.begin(); }

  vector<MyBasicBlock *>::iterator end() { return BBs.end(); }

  vector<MyLoop *> &getLoops() { return Loops; }

  MyLoop *getLoop(unsigned i) { return Loops[i]; }

  string getDependencyForConstant(Constant *cst);

  MyModuleContext &getModuleContext() { return MMC; }

  void loopAnalysis();

  void applyInstrumentations();

  MTSNode *getMTSNodeFromValue(Value *v) {
    auto it = V2MTS.find(v);
    if (it == V2MTS.end()) {
      v->print(errs());
      errs() << "\n";
      errs() << "Value doesn't exist in V2MTS.\n";
      exit(1);
    } else {
      return it->second;
    }
  }

  size_t getNumArgs() { return Args.size(); }

  Function &getFunction() { return TheFunction; }

  const unsigned getID() { return ID; }

  void setAttr(AttrBase *p) { Attr = p; }

  AttrBase *getAttr() { return Attr; }

  bool isMemChange() { return MemChange; }

  friend class MyBasicBlock;
};

} // namespace llvm

#endif
