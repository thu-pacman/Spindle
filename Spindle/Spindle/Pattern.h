#ifndef PATTERN_H
#define PATTERN_H

using namespace llvm;
using namespace std;

namespace llvm {
// Dependence tree node.
class DTNode {};

// Dependence tree root
class DTRoot : DTNode {
  DTNode *node;

  Instruction *inst;

public:
  // Dump as string.
  string dump() {
    string s;
    return s;
  }
};

// Dependence tree leaf node.
class DTLeafNode : public DTNode {};

// Dependence tree leaf node, constant value
class DTCV : public DTLeafNode {};

// Dependence tree leaf node, base memory address
class DTBM : public DTLeafNode {};

// Dependence tree leaf node, function parameter
class DTFP : public DTLeafNode {};

// Dependence tree leaf node, data-dependent value
class DTDV : public DTLeafNode {};

// Dependence tree leaf node, loop induction variable
class DTLI : public DTLeafNode {};

// Dependence tree non-leaf node.
class DTOperatorNode : public DTNode {};

// Dependence tree non-leaf node, add
class DTAdd : public DTOperatorNode {};

// Dependence tree non-leaf node, sub
class DTSub : public DTOperatorNode {};

// Dependence tree non-leaf node, mul
class DTMul : public DTOperatorNode {};

// Dependence tree non-leaf node, div
class DTDiv : public DTOperatorNode {};

} // namespace llvm

#endif
