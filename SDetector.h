#ifndef SDETECTOR_H
#define SDETECTOR_H
#include "MTS.h"
#include <set>

using namespace llvm;
using namespace std;

namespace llvm {
class SDFunctionAttr : public AttrBase {
  set<unsigned> BasementAid;
};

class SDetector {
  MyModuleContext &MMC;

  void collectBasementArg();

public:
  SDetector(MyModuleContext &MMC) : MMC(MMC) { runDetector(); }

  void runDetector();

  void analysisLoad(MTSLoad *ln);

  void analysisStore(MTSStore *sn);

  void analysisMemAccess(MTSMemAccess *mn);

  static void instrumentForMalloc(MTSNode *node);

  static void instrumentForRealloc(MTSNode *node);

  static void instrumentForCalloc(MTSNode *node);

  static void instrumentForFree(MTSNode *node);
};

} // namespace llvm

#endif // SDETECTOR_H
