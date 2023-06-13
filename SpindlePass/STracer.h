#pragma once

#include <fstream>

#include "MAS.h"
#include "instrument.h"

using namespace llvm;

namespace llvm {

class DTraceParser {
    std::ifstream dtrace;

public:
    explicit DTraceParser(const std::string &fname);

    auto parseBr() -> bool;
    auto parseValue() -> ValueType;
};

class STracer {
    MASModule &MAS;

public:
    explicit STracer(MASModule &MAS);

    void run(InstrumentationBase *instrument, bool fullMem, bool fullBr);
    void replay(Function *func,
                DTraceParser &dtrace,
                raw_fd_ostream &out,
                InstrumentationDummy *instrument,
                SymbolTable &table);
};

}  // namespace llvm
