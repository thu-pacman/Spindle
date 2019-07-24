#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include "MTS.h"

using namespace llvm;
using namespace std;

namespace llvm {
class Instrumentation {
  static void instrumentForMalloc(MTSNode *node);

  static void instrumentForRealloc(MTSNode *node);

  static void instrumentForCalloc(MTSNode *node);

  static void instrumentForFree(MTSNode *node);
};
} // namespace llvm

#endif
