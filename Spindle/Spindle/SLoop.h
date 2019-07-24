#ifndef LLVM_SLOOP_H
#define LLVM_SLOOP_H

#include "llvm/Analysis/LoopPass.h"

using namespace llvm;
using namespace std;

namespace llvm {
// class LoopIndvar
// Record one of the induction variables for a specific Loop Loop.
// A loop may contain one or more loop induction variables.
//
class LoopIndvar {
  MTSPhiNode &PHI;

  SLoop *SL;

  Value *Init;

  Value *Fini;

  Pattern Step;

  char OpSymbol;

  int Increasement;

public:
  LoopIndvar(MTSPhiNoce &PHI) : PHI(PHI) { PHI.setIndvarOf(this); }

  LoopIndvar(MTSPhiNode &PHI, SLoop *SL, char OpSymbol, int Incresement)
      : PHI(PHI), SL(SL), OpSymbol(OpSymbol), Incresement(Incresement) {
    // errs() << "set phi as indvar\n";
    PHI.setIndvarOf(this);
  }

  SLoop *getMyLoop() { return SL; }
};

class SLoop {
  Loop &TheLoop;

  SFunction &Parent;

  unsigned Depth;

  const unsigned ID;

  static SLoopID = 0;

  bool Analyzed;

  vector<LoopIndvar *> Indvars;

  BasicBlock *preheader;

  BasicBlock *latch;

  BasicBlock *header;

  SmallVector<BasicBlock *, 8> exitBlocks;

  SmallVector<BasicBlock *, 8> exitingBlocks;

  BasicBlock *exitBlock;

  BasicBlock *exitingBlock;

public:
  SLoop(Loop &L, SFunction &Parent)
      : TheLoop(L), ID(SLoopID++), Parent(Parent), Analyzed(false) {
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

  int getLoopDepth() { return Depth; }
};
} // namespace llvm

#endif
