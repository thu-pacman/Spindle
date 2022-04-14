#include "AST.h"

ASTAbstractNode *NIL = new ASTAbstractNode;

void ASTAbstractNode::print() {}

void ASTOpNode::print() {
    errs() << "( " << opCode << ' ';
    lc->print();
    errs() << ", ";
    rc->print();
    errs() << ')';
}

void ASTLeafNode::print()  {
    errs() << '(' << *v << ')';
}