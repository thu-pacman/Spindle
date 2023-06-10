#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <utility>

#include "AST.h"
#include "MAS.h"
#include "llvm/IR/InstVisitor.h"
#include "utils.h"

using namespace llvm;

class MASModule;

class FormulaVisitor : public InstVisitor<FormulaVisitor, ASTAbstractNode *> {
    std::function<bool(Value *)> leafChecker;
    bool debug;
    DataLayout DL;
    LLVMContext *context;

public:
    FormulaVisitor(decltype(leafChecker) leafComputableChecker,
                   MASModule *M,
                   bool debug = false);

    auto visitValue(Value *v) -> ASTAbstractNode *;
    auto visitInstruction(Instruction &I) -> ASTAbstractNode *;
    auto visitUnaryInstruction(UnaryInstruction &UI) -> ASTAbstractNode *;
    auto visitBinaryOperator(BinaryOperator &BOI) -> ASTAbstractNode *;
    auto visitGetElementPtrInst(GetElementPtrInst &GEPI) -> ASTAbstractNode *;
};

struct InstrMetaInfo;

class MemDependenceVisitor : public InstVisitor<MemDependenceVisitor> {
    DenseMap<Instruction *, InstrMetaInfo> &meta;
    std::set<Value *> &indVars;

public:
    MemDependenceVisitor(decltype(meta) &meta, decltype(indVars) &indVars);

    void visitLoadInst(LoadInst &LI);
    void visitStoreInst(StoreInst &SI);
    void visitGetElementPtrInst(GetElementPtrInst &GEPI);
    void visitInstruction(Instruction &I);
};

template <class T>
class ASTVisitor {
public:
    auto dispatch(ASTAbstractNode *v) -> T;
    virtual auto visit(ASTOpNode *v) -> T = 0;
    virtual auto visit(ASTLeafNode *v) -> T = 0;
};

class InstrumentationVisitor : public ASTVisitor<void> {
    std::function<void(ASTLeafNode *)> leafInstrumentation;

public:
    explicit InstrumentationVisitor(decltype(leafInstrumentation) leafFunc);

    void visit(ASTOpNode *v) override;
    void visit(ASTLeafNode *v) override;
};

class CalculationVisitor : public ASTVisitor<ValueType> {
    SymbolTable &table;

public:
    explicit CalculationVisitor(SymbolTable &table);

    auto visit(ASTOpNode *v) -> ValueType override;
    auto visit(ASTLeafNode *v) -> ValueType override;
};