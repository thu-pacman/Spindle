#pragma once
#include "MAS.h"

using namespace llvm;
using namespace std;

namespace llvm {

class STracer {
    MASModule &MAS;

  public:
    explicit STracer(MASModule &MAS) : MAS(MAS) {}
    void print();
};

} // namespace llvm
