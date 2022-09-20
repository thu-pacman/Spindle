#pragma once

#include <map>
#include <set>
#include <functional>
#include <utility>

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
        : leafChecker(std::move(leafComputableChecker)), debug(debug) {
    }
    auto visitValue(Value *v) -> ASTAbstractNode *;
    auto visitInstruction(Instruction &I) -> ASTAbstractNode *;
    auto visitUnaryInstruction(UnaryInstruction &UI) -> ASTAbstractNode *;
    auto visitBinaryOperator(BinaryOperator &BOI) -> ASTAbstractNode *;
    auto visitGetElementPtrInst(GetElementPtrInst &GEPI) -> ASTAbstractNode *;
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
