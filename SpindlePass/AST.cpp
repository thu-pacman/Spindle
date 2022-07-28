#include "AST.h"

#include "llvm/IR/Instruction.h"

void ASTAbstractNode::print(raw_fd_ostream &out) {
}

void ASTOpNode::print(raw_fd_ostream &out) {
    out << '(';
    lc->print(out);
    out << ") ";
    switch (opCode) {
    case Instruction::Add:
    case Instruction::FAdd:
        out << '+';
        break;
    case Instruction::Sub:
    case Instruction::FSub:
        out << '-';
        break;
    case Instruction::Mul:
    case Instruction::FMul:
        out << '*';
        break;
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::FDiv:
        out << '/';
        break;
    case Instruction::URem:
    case Instruction::SRem:
    case Instruction::FRem:
        out << '%';
        break;
    case Instruction::Shl:
        out << "<<";
        break;
    case Instruction::LShr:
        out << ">>";
        break;
    case Instruction::AShr:
        out << ">>>";
        break;
    case Instruction::And:
        out << "&";
        break;
    case Instruction::Or:
        out << "|";
        break;
    case Instruction::Xor:
        out << "^";
        break;
    default:
        errs() << "Unknown opcode for BinaryOperator: " << opCode << "\n";
        exit(1);
    }
    out << " (";
    rc->print(out);
    out << ')';
}

void ASTLeafNode::print(raw_fd_ostream &out) {
    out << *v;
}