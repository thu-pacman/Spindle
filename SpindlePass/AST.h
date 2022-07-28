#pragma once
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct ASTAbstractNode {
    bool computable = true;
    virtual void print(raw_fd_ostream &out);
};

struct ASTOpNode : public ASTAbstractNode {
    ASTAbstractNode *lc, *rc;
    unsigned opCode;
    void print(raw_fd_ostream &out) override;
};

struct ASTLeafNode : public ASTAbstractNode {
    Value *v;
    void print(raw_fd_ostream &out) override;
};