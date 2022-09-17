#pragma once

#include <map>
#include <set>
#include <functional>

#include "AST.h"
#include "MAS.h"
#include "llvm/IR/InstVisitor.h"

using namespace llvm;
using std::map;
using std::set;
using std::function;

class ASTVisitor : public InstVisitor<ASTVisitor, ASTAbstractNode *> {
    function<bool(Value *)> leafChecker;
    bool debug;

public:
    explicit ASTVisitor(function<bool(Value *)> leafComputableChecker,
                        bool debug = false)
        : leafChecker(leafComputableChecker), debug(debug) {
    }
    ASTAbstractNode *visitValue(Value *v);
    ASTAbstractNode *visitInstruction(Instruction &I);
    ASTAbstractNode *visitUnaryInstruction(UnaryInstruction &UI);
    ASTAbstractNode *visitBinaryOperator(BinaryOperator &BOI);
    ASTAbstractNode *visitGetElementPtrInst(GetElementPtrInst &GEPI);
};

struct InstrMetaInfo;

class GEPDependenceVisitor : public InstVisitor<GEPDependenceVisitor> {
    map<Instruction *, InstrMetaInfo> &meta;
    set<Value *> &indVars;

public:
    explicit GEPDependenceVisitor(map<Instruction *, InstrMetaInfo> &meta,
                                  set<Value *> &indVars)
        : meta(meta), indVars(indVars) {
    }
    void visitGetElementPtrInst(GetElementPtrInst &GEPI);
    void visitInstruction(Instruction &I);
};
