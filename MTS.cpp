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

// for debug
// TODO: Delete it. Change myassert to assert.
// static void myassert(string &s) {
static void myassert(char *s) {
  // errs() << s << "\n";
  // eixt(1);
}

// Functional function.

void printDebugLocIfPosibble(Value *v) {
  if (auto inst = dyn_cast<Instruction>(v)) {
    auto dl = inst->getDebugLoc();
    errs() << "file: " << inst->getModule()->getName().str() << ",   "
           << inst->getFunction()->getName().str();
    if (dl) {
      errs() << ",    row: " << dl.getLine() << ",  col: " << dl.getCol();
    }
    errs() << "\n";
  }
}

static string getStringFromValue(const Value *v) {
  string str;
  raw_string_ostream rso(str);
  v->print(rso);
  return str;
}

static string num2string(int i) {
  ostringstream oss;
  oss << i;
  return oss.str();
}

static string getPatternForGEP(User *usr, vector<string> &op_pattern,
                               DataLayout *DL) {
  ostringstream oss;
  uint64_t offset = 0;
  unsigned num_op = 0;
  Type *elemTy = usr->getOperand(0)->getType();
  auto GTI = gep_type_begin(usr), GTE = gep_type_end(usr);
  // gep_type start from Operand(1). Operand(0) is considered as a base address.
  // The rest is used for compute the offset.
  for (auto i = 1; GTI != GTE; ++GTI, ++i) {
    Value *idx = GTI.getOperand();
    string &pat = op_pattern[i];
    if (StructType *sTy = GTI.getStructTypeOrNull()) {
      if (!idx->getType()->isIntegerTy(32)) {
        oss << "u";
        // myassert("Type error!");
        break;
      }
      unsigned fieldNo = cast<ConstantInt>(idx)->getZExtValue();
      const StructLayout *SL = DL->getStructLayout(sTy);
      offset += SL->getElementOffset(fieldNo);
    } else {
      if (ConstantInt *csti = dyn_cast<ConstantInt>(idx)) {
        uint64_t arrayIdx = csti->getSExtValue();
        offset += arrayIdx * DL->getTypeAllocSize(GTI.getIndexedType());
      } else {
        if (offset != 0) {
          oss << "c" << offset;
          offset = 0;
          ++num_op;
        }
        oss << "*2c" << DL->getTypeAllocSize(GTI.getIndexedType()) << pat;
        ++num_op;
      }
    }
  }

  if (offset != 0) {
    oss << "c" << offset;
    offset = 0;
    ++num_op;
  }

  auto vs = oss.str();
  oss.str("");
  oss.clear();
  if (num_op) {
    oss << "+" << (num_op + 1) << op_pattern[0] << vs;
  } else {
    oss << op_pattern[0];
  }

  return oss.str();
}

// Replace all the string "pat" with "rep" in "str"
static unsigned findAndReplaceAll(string &str, string pat, string rep) {
  unsigned cnt = 0;
  auto pos = str.find(pat);
  while (pos != string::npos) {
    ++cnt;
    str.replace(pos, pat.size(), rep);
    pos = str.find(pat);
  }
  return cnt;
}

// TODO: Are all the binary operator symbols included?
static bool isBinaryOpSymbol(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' ||
         c == '^' || c == '%' || '<' || '>';
}

// If a string can be converted to a integer.
static bool isInteger(string &str) {
  auto pos = 0;
  if (str[0] == '-' || str[0] == '+')
    pos = 1;
  for (auto sz = str.size(); pos != sz; ++pos)
    if (!isdigit(str[pos]))
      return false;
  return true;
}

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

void MTSUnhandled::computePattern(vector<string> &args) {}

// TODO: Automicly read opcode from llvm/include/llvm/IR/Instruction.def
// TODO: More specific symble. Like \add and \fadd, \shift_right logically and
// arithmeticly. But use llvm-defined opcode is not a good idea, because there
// are some instructions like GetElementPtr should do the instruction analysis
// with llvm context, which cannot be left to runtime or post-runtime. Maybe a
// good way to do this is to get a self-defined opcode list.
void MTSBinaryOp::computePattern(vector<string> &args) {
  ostringstream oss;
  auto bo = dyn_cast<BinaryOperator>(getInstruction());
  auto opcode = bo->getOpcode();
  switch (opcode) {
  case 11:
  case 12:
    oss << "+";
    break;
  case 13:
  case 14:
    oss << "-";
    break;
  case 15:
  case 16:
    oss << "*";
    break;
  case 17:
  case 18:
  case 19:
    oss << "/";
    break;
  case 20:
  case 21:
  case 22:
    oss << "%%";
    break;
  // TODO: Logical shift and arithmetic shift
  case 23:
    oss << "<";
    break;
  case 24:
  case 25:
    oss << ">";
    break;
  case 26:
    oss << "&";
    break;
  case 27:
    oss << "|";
    break;
  case 28:
    oss << "^";
    break;
  default:
    oss << "opcode: " << opcode;
    break;
  }
  oss << args[0] << args[1];
  setPattern(oss.str());
}

// TODO: Instrument alloc and free functions to record the allocated addresses
// and freed addresses.
// Including: malloc, calloc, realloc, free,
//            ZdaPv, ZdaPvRKSt9nothrow_t, ZdlPv, ZdlPvPKSt9nothrow_t (delete)
//            Znaj, ZnajRKSt9nothrow_t, Znam, ZnamRKSt9nothrow_t, Znwj,
//            ZnwjRKSt9nothrow_t, Znwm, ZnwmRKSt9nothrow_t (new)
// See http://legup.eecg.utoronto.ca/doxygen-4.0/namespacellvm_1_1LibFunc.html
// for c++ operators new and delete.
void MTSCall::computePattern(vector<string> &args) {
  // errs() << getStringFromValue(getInstruction()) << "\n";
  // errs() << args[0] << "\n";
  if (args.size() != 1) {
    // errs() << getStringFromValue(getInstruction()) << "\n";
    // myassert(getStringFromValue(getInstruction()) + "\n" +
    //          "Strange function call.");
  } else
    setPattern(args[0]);
}

void MTSCmp::computePattern(vector<string> &args) {}

void MTSGetElementPtr::computePattern(vector<string> &args) {
  auto usr = dyn_cast<User>(getInstruction());
  auto pat = getPatternForGEP(
      usr, args, getParent().getParent().getModuleContext().getDataLayout());
  setPattern('e' + pat);
  // setPattern(pat);
}

// For exception handle.
void MTSLandingPad::computePattern(vector<string> &args) {}

void MTSPhiNode::computePattern(vector<string> &args) {
  string pat = 'p' + to_string(getID());
  setPattern(pat);
}

MyLoop *MTSPhiNode::tryToGetMyLoop() {
  if (IndvarOf) {
    return IndvarOf->getMyLoop();
  }
  return nullptr;
}

void MTSSelect::computePattern(vector<string> &args) {}

void MTSStore::computePattern(vector<string> &args) {
  string pat = "s";
  analysisMemAccess(dyn_cast<StoreInst>(getInstruction())->getPointerOperand());
  if (args.size() == 2) {
    pat = pat + args[1];
    setPattern(pat);
  } else {
    errs() << "StoreInst must have 2 operands.\n";
    getInstruction()->print(errs());
    errs() << "\n";
    exit(1);
  }
}

void MTSReturn::computePattern(vector<string> &args) {}

// TODO: If this switch is for loop?
// TODO: Assert the cases must be constants.
void MTSSwitch::computePattern(vector<string> &args) {
  string pat = 'w' + to_string(args.size());
  for (auto &s : args)
    pat += s;
  setPattern(pat);
}

// TODO: If this branch is for loop?
void MTSBranch::computePattern(vector<string> &args) {
  // If sz == 1, this is an unconditional branch.
  // If sz == 3, this is a conditional branch.
  string pat = 'j' + to_string(args.size());
  for (auto &s : args)
    pat += s;
  setPattern(pat);
}

// TODO: Record the allocated address here.
void MTSAlloca::computePattern(vector<string> &args) {}

void MTSCast::computePattern(vector<string> &args) {
  if (args.size() != 1) {
    // CastInst should have only 1 operand.
  } else
    setPattern(args[0]);
}

void MTSLoad::computePattern(vector<string> &args) {
  string pat = "l";
  analysisMemAccess(dyn_cast<LoadInst>(getInstruction())->getPointerOperand());
  if (args.size() == 1) {
    pat = pat + args[0];
    setPattern(pat);
  } else {
    // LoadInst must have 1 operand.
  }
}

static void DFSAnalysisMemAccess(Value *v, vector<Value *> &deps) {
  if (LoadInst::classof(v) || Argument::classof(v) || PHINode::classof(v)) {
    deps.push_back(v);
  } else if (auto usr = dyn_cast<User>(v)) {
    for (size_t i = 0, opnum = usr->getNumOperands(); i < opnum; ++i) {
      auto op = usr->getOperand(i);
      if (!Constant::classof(op)) {
        deps.push_back(op);
        DFSAnalysisMemAccess(op, deps);
      }
    }
  } else {
    errs() << "DFSAnalysisMemAccess: ";
    v->print(errs());
    errs() << "\n";
  }
}

void MTSMemAccess::analysisMemAccess(Value *addr) {
  while (auto ci = dyn_cast<BitCastOperator>(addr)) {
    addr = ci->getOperand(0);
  }
  while (auto ci = dyn_cast<CastInst>(addr)) {
    addr = ci->getOperand(0);
  }
  if (auto gep = dyn_cast<GEPOperator>(addr)) {
    Basement = gep->getPointerOperand();
    for (size_t i = 1, opnum = gep->getNumOperands(); i < opnum; ++i) {
      DFSAnalysisMemAccess(gep->getOperand(i), Dependencies);
    }
  } else if (auto ai = dyn_cast<AllocaInst>(addr)) {
    Basement = ai;
    // IsConstant = true;
  } else if (auto gv = dyn_cast<GlobalValue>(addr)) {
    Basement = gv;
    // IsConstant = true;
  } else if (auto arg = dyn_cast<Argument>(addr)) {
    Basement = arg;
  } else if (auto phi = dyn_cast<PHINode>(addr)) {
    Basement = phi;
    FullInstrumentation = true;
  } else if (auto sel = dyn_cast<SelectInst>(addr)) {
    Basement = sel;
    FullInstrumentation = true;
  } else if (auto ci = dyn_cast<CallInst>(addr)) {
    Basement = ci;
    FullInstrumentation = true;
  } else if (auto li = dyn_cast<LoadInst>(addr)) {
    Basement = li;
    FullInstrumentation = true;
  } else {
    // errs() << "Other type of memory access: ";
    // getInstruction()->print(errs());
    // errs() << " ------ ";
    // addr->print(errs());
    // errs() << " ------ ";
    // printDebugLocIfPosibble(getInstruction());
    // errs() << "\n";
    FullInstrumentation = true;
  }
  // errs() << "MemAccess: ";
  // addr->print(errs());
  // errs() << "\n";
  // errs() << "Basement: ";
  // Basement->print(errs());
  // errs() << "\n";
}

bool MTSMemAccess::isDependencyLoopIndvar(Value *v) {
  if (auto phi = dyn_cast<PHINode>(v)) {
    auto node =
        dyn_cast<MTSPhiNode>(getParent().getParent().getMTSNodeFromValue(v));
    return node->isLoopIndvar();
  }
  return false;
}

MyLoop *MTSMemAccess::tryToGetMyLoopFor(Value *v) {
  if (auto phi = dyn_cast<PHINode>(v)) {
    auto node =
        dyn_cast<MTSPhiNode>(getParent().getParent().getMTSNodeFromValue(v));
    return node->tryToGetMyLoop();
  }
  return nullptr;
}

// class MyModuleContext
// =========================================================

void MyModuleContext::addFunction(MyFunction *MF) {
  FunctionList.push_back(MF);
  FuncsMap.insert({&MF->getFunction(), MF->getID()});
}

// class MyLoop ================================================================
static LoopIndvar *getLoopIndvar(MTSPhiNode *pnode) {
  auto pat = pnode->getPatternString();
  auto pid = pnode->getID();
  findAndReplaceAll(pat, 'p' + to_string(pid), "");
  // errs() << pat << "\n";
  return nullptr;
}

void MyLoop::computeIndvars() {
  auto preheader = TheLoop.getLoopPreheader();
  auto header = TheLoop.getHeader();
  auto latch = TheLoop.getLoopLatch();
  SmallVector<BasicBlock *, 8> exitingBlocks;
  TheLoop.getExitingBlocks(exitingBlocks);
  // myassert(preheader && header && latch && !exitingBlocks.empty() && "Wrong
  // loop structure.");
  if (!preheader || !header || !latch || exitingBlocks.empty())
    return;
  // FIXME: function classof cannot take a iterator as the parameter?
  // TheLoop.print(errs());
  for (auto inst = header->begin();
       inst != header->end() && PHINode::classof(&(*inst)); ++inst) {
    auto phi = dyn_cast<PHINode>(inst);
    MTSPhiNode *mtsphi = (MTSPhiNode *)Parent.getMTSNodeFromValue(phi);
    // if (auto iv = getLoopIndvar(mtsphi))
    //   Indvars.push_back(iv);
    if (phi->getNumIncomingValues() == 2) {
      auto vpreheader = phi->getIncomingValueForBlock(preheader),
           vlatch = phi->getIncomingValueForBlock(latch);
      if (!vpreheader || !vlatch)
        continue;
      // TODO: vlatch is Constant
      if (Constant::classof(vlatch))
        continue;
      // TODO: vlatch is Argument
      if (Argument::classof(vlatch))
        continue;
      auto MTSLatch = Parent.getMTSNodeFromValue(vlatch);
      auto pat = MTSLatch->getPatternString();
      // Filter the pattern for PhiNode.
      // Currently only 1 PhiNode pattern is allowed.
      // TODO: More scalable.
      // errs() << pat << "\n";
      auto cnt = findAndReplaceAll(pat, 'p' + to_string(mtsphi->getID()), "");
      if (cnt <= 1 && !Pattern::isConstantPattern(pat))
        continue;
      // MTSLatch->getInstruction()->print(errs());
      // errs() << "Loop indvar pattern: " << pat << "\n";
      // Current asume that the pattern should be (op_symbol) + (constant)
      char op_symbol = pat[0];
      if (!isBinaryOpSymbol(op_symbol))
        continue;
      auto pos = pat.find('c');
      // string numstr = pat;
      if (pos == string::npos)
        continue;
      auto numstr = pat.substr(pos + 1);
      // TODO: float?
      if (!isInteger(numstr))
        continue;
      if (numstr == "4294967295")
        numstr = "-1";
      auto num = stoi(numstr);
      if (num < 0)
        if (op_symbol == '+') {
          op_symbol = '-';
          num = -num;
        } else if (op_symbol == '-') {
          op_symbol = '+';
          num = -num;
        }
      // errs() << op_symbol << "  " << num << "\n";
      auto *idv = new LoopIndvar(*mtsphi, this, op_symbol, num);
      Indvars.push_back(idv);
      // errs() << "Indvar Added\n";
      // TODO: Add to Indvar
      // TODO: Is Indvar?
      // string phiID = "p" + to_string(mtsphi->getID());
      // auto phiID = mtsphi->getID();
      // for (size_t i = 0, sz = pat.size(); i < sz; ++i) {
      //   if (pat[i] == 'p') {
      //     if (pat.substr(i + 1, i + phiID.size() + 1) == phiID) {
      //
      //     } else {
      //       break;
      //     }
      //   }
      // }
      // TODO: Delete the PhiNode Pattern.
      // deleteSubStr(pat, phiID);
      // if (pat.isConstant())
      //   Indvars.push_back(LoopIndvar(*mtsphi));
    }
  }

  // Set loop as analyzed.
  Analyzed = true;
} // namespace llvm

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
void MyFunction::dependenceAnalysis() {
  for (auto mbb : BBs) {
    for (auto node : mbb->MTS) {
      getDependency(node);
    }
  }
}

// TODO: Handle all the Users.
void MyFunction::getDependency(MTSNode *node) {
  auto inst = node->getInstruction();
  vector<string> operands_patterns;
  for (auto op = inst->op_begin(); op != inst->op_end(); ++op) {
    auto v = op->get();
    // Value -- Argument, BasicBlock, InlineAsm, MetadataAsValue, User
    if (auto arg = dyn_cast<Argument>(v)) {
      auto it = find(Args.begin(), Args.end(), arg);
      // if (it == Args.end())
      //   myassert("Argument does not exist.");
      auto idx = distance(Args.begin(), it);
      // idx is the index of this argument.
      operands_patterns.push_back("a" + num2string(idx));
    } else if (auto bb = dyn_cast<BasicBlock>(v)) {
      auto it = find_if(BBs.begin(), BBs.end(),
                        [&](MyBasicBlock *mbb) { return &mbb->getBB() == bb; });
      // if (it == BBs.end())
      //   myassert("BasicBlock does not exist.");
      auto idx = distance(BBs.begin(), it);
      // idx is the index of this BasicBlock in MyBasicBlock vector.
      operands_patterns.push_back("b" + num2string(idx));
    } else if (auto f = dyn_cast<Function>(v)) {
      auto FM = MMC.getFunctionMap();
      auto it = FM.find(f);
      if (it == FM.end()) {
        operands_patterns.push_back("f" + num2string(-1));
        // errs() << "No such function: " << getStringFromValue(v) << "\n";
        // exit(1);
        // myassert(getStringFromValue(v) + "\n" + "No such function.");
      } else {
        auto idx = it->second;
        operands_patterns.push_back("f" + num2string(idx));
      }
    } else if (auto ia = dyn_cast<InlineAsm>(v)) {
      // errs() << getStringFromValue(v) << "\n";
      // myassert(getStringFromValue(v) + "\n" + "InlineAsm.");
    } else if (auto mdv = dyn_cast<MetadataAsValue>(v)) {
      // errs() << getStringFromValue(v) << "\n";
      // myassert(getStringFromValue(v) + "\n" + "MetadataAsValue.");
    }
    // User -- Constant, DerivedUser, Instruction, Operator
    else if (auto cst = dyn_cast<Constant>(v)) {
      operands_patterns.push_back(getDependencyForConstant(cst));
    } else if (auto ins = dyn_cast<Instruction>(v)) {
      if (auto li = dyn_cast<LoadInst>(ins)) {
        string str = 'l' + to_string(V2MTS[v]->getID());
        // li->print(errs());
        // errs() << "load from: " << str << "\n";
        operands_patterns.push_back(str);
      } else {
        string str = V2MTS[v]->getPatternString();
        // TODO: What if the size of pattern is too long?
        if (str.size() > 32)
          str = 'i' + to_string(V2MTS[v]->getID());
        // operands_patterns.push_back(V2MTS[v]->getPatternString());
        operands_patterns.push_back(str);
      }
    }
    // Some DerivedUsers are also Instructions, so Instruction must be handled
    // before DerivedUser.
    // TODO: Do all the DerivedUsers are Instructions?
    else if (auto drv = dyn_cast<DerivedUser>(v)) {
      errs() << "DerivedUser: " << getStringFromValue(v) << "\n";
      exit(1);
      // myassert("DerivedUser.");
    } else if (auto oprt = dyn_cast<Operator>(v)) {
      errs() << "Operator: " << getStringFromValue(v) << "\n";
      exit(1);
      // myassert(getStringFromValue(v) + "\n" + "Operator.");
    }
    // Should be non-reachable.
    else {
      // errs() << getStringFromValue(v) << "\n";
      // myassert("No such type of Value.\n");
      auto idx = MMC.getOrInsertUnhandled(cst);
      string str = 'u' + to_string(idx);
      operands_patterns.push_back(str);
    }
  }
  // TODO: Compute the pattern after analysis, only compute the pattern for the
  // ones that needed.
  node->computePattern(operands_patterns);
}

string MyFunction::getDependencyForConstant(Constant *cst) {
  ostringstream oss;
  // errs() << getStringFromValue(cst) << "\n";
  if (auto CI = dyn_cast<ConstantInt>(cst)) {
    if (CI->getBitWidth() <= 64)
      oss << "c" << CI->getSExtValue();
    // The expression above will throw an error when the bitwidth of this
    // ConstantInt is larger than 64.
    // TODO: Is this ConstantInt signed?
    else
      oss << "c" << CI->getValue().toString(10, true);
  } else if (auto CE = dyn_cast<ConstantExpr>(cst)) {
    if (CE->isGEPWithNoNotionalOverIndexing()) {
      vector<string> op_patterns;
      for (auto it = CE->op_begin(), ope = CE->op_end(); it != ope; ++it) {
        if (auto cst = dyn_cast<Constant>(*it)) {
          op_patterns.push_back(getDependencyForConstant(cst));
        } else {
          // myassert("Non-constant in a ConstantExpr.");
        }
      }
      auto pat = getPatternForGEP(cst, op_patterns, MMC.getDataLayout());
      oss << 'e' << pat;
    } else if (CE->isCast()) {
      auto op = CE->getOperand(0);
      if (auto cst = dyn_cast<Constant>(op)) {
        auto pat = getDependencyForConstant(cst);
        oss << pat;
      } else {
        // myassert("Non-constant in a ConstantExpr.");
      }
    } else {
      // errs() << "ConstantExpr: " << getStringFromValue(CE) << "\n";
      auto idx = MMC.getOrInsertUnhandled(cst);
      oss << "u" << idx;
      // myassert(getStringFromValue(CE) + "\n" + "ConstantExpr but not
      // handled.");
    }
  } else if (auto GVar = dyn_cast<GlobalVariable>(cst)) {
    auto &GM = MMC.getGlobalMap();
    auto it = GM.find(GVar);
    if (it == GM.end()) {
      // errs() << getStringFromValue(cst) << "\n";
      // myassert(getStringFromValue(cst) + "\n" +
      //          "GlobalVariable is not in list.");
    } else {
      auto idx = it->second;
      // idx is the index of this GlobalVariable.
      oss << "g" << idx;
    }
  } else {
    // Handle unhandled value.
    // TODO: Is there any other values can be handled here?
    auto idx = MMC.getOrInsertUnhandled(cst);
    oss << "u" << idx;
  }

  return oss.str();
}

void MyFunction::loopAnalysis() {
  for (auto ML : Loops) {
    ML->computeIndvars();
  }
}

void MyFunction::applyInstrumentations() {
  for (auto MBB : BBs) {
    for (auto node : *MBB) {
      // auto Kind = node->getKind();
      for (auto instrmt : node->getInstrumentations()) {
        instrmt->doInstrumentation(node);
      }
    }
  }
}

} // namespace llvm
