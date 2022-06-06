#include "AST.h"

ASTAbstractNode *NIL = new ASTAbstractNode;

void ASTAbstractNode::print(raw_fd_ostream &out) {}

void ASTOpNode::print(raw_fd_ostream &out) {
    out << "( " << opCode << ' ';
    lc->print(out);
    out << ", ";
    rc->print(out);
    out << ')';
}

void ASTLeafNode::print(raw_fd_ostream &out)  {
    out << '(' << *v << ')';
}