#include "MTS.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/User.h"
#include <sstream>
#include <string>

using namespace llvm;
using namespace std;

namespace llvm {

// // Remove all the substring sub containing in str.
// void deleteSubStr(string &str, const string &sub) {
//   bool flag = true;
//   auto subsz = sub.size();
//   while (flag) {
//     auto pos = str.find(sub);
//     if (pos == string::npos)
//       flag = false;
//     else
//       str.erase(pos, pos + subsz);
//   }
// }

// class MTSNode ===============================================================

// class MyModuleContext
// =========================================================

void MyModuleContext::addFunction(MyFunction *MF) {
  FunctionList.push_back(MF);
  FuncsMap.insert({&MF->getFunction(), MF->getID()});
}

// class MyLoop ================================================================

// class MyBasicBlock ==========================================================
void MyBasicBlock::buildMTS() {
  for (auto &inst : TheBB) {
    // Different types of instructions make different MTS.
    MTSNode *node;
    if (auto BOI = dyn_cast<BinaryOperator>(&inst))
      node = new MTSBinaryOp(*BOI, *this);
    else if (auto CI = dyn_cast<CallInst>(&inst))
      node = new MTSCall(*CI, *this);
    else if (auto CI = dyn_cast<CmpInst>(&inst))
      node = new MTSCmp(*CI, *this);
    else if (auto GEP = dyn_cast<GetElementPtrInst>(&inst))
      node = new MTSGetElementPtr(*GEP, *this);
    else if (auto LP = dyn_cast<LandingPadInst>(&inst))
      node = new MTSLandingPad(*LP, *this);
    else if (auto PHI = dyn_cast<PHINode>(&inst))
      node = new MTSPhiNode(*PHI, *this);
    else if (auto SI = dyn_cast<SelectInst>(&inst))
      node = new MTSSelect(*SI, *this);
    else if (auto SI = dyn_cast<StoreInst>(&inst))
      node = new MTSStore(*SI, *this);
    else if (auto RI = dyn_cast<ReturnInst>(&inst))
      node = new MTSReturn(*RI, *this);
    else if (auto SI = dyn_cast<SwitchInst>(&inst))
      node = new MTSSwitch(*SI, *this);
    else if (auto BI = dyn_cast<BranchInst>(&inst))
      node = new MTSBranch(*BI, *this);
    else if (auto AI = dyn_cast<AllocaInst>(&inst))
      node = new MTSAlloca(*AI, *this);
    else if (auto CI = dyn_cast<CastInst>(&inst))
      node = new MTSCast(*CI, *this);
    else if (auto LI = dyn_cast<LoadInst>(&inst))
      node = new MTSLoad(*LI, *this);
    else
      node = new MTSUnhandled(inst, *this);
    MTS.push_back(node);
    Parent.V2MTS.insert(
        map<Value *, MTSNode *>::value_type(dyn_cast<Value>(&inst), node));
  }
}

// class MyFunction ============================================================

} // namespace llvm
