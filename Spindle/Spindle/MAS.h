#pragma once

#include <llvm/Analysis/LoopInfo.h>

#include <map>
#include <set>
#include <vector>

#include "AST.h"
#include "visitor.h"

using namespace llvm;

using std::map;
using std::set;
using std::vector;

class MASLoop {
    struct LoopIndVar {
        Value *initValue, *finalValue;
        ASTAbstractNode *delta;
    };

    Loop &loop;
    vector<LoopIndVar> indVars;

public:
    explicit MASLoop(Loop &loop) : loop(loop) {
    }

    void analyze(set<Value *> &parentIndVars);

    auto &getIndVars() {
        return indVars;
    }
};

struct InstrMetaInfo {
    bool isGEPDependence;
    MASLoop *loop;

    InstrMetaInfo() : isGEPDependence(false), loop(nullptr) {
    }
};

class MASFunction {
    Function &func;
    vector<MASLoop *> loops;
    set<Value *> indVars;
    map<Instruction *, InstrMetaInfo> meta;

    void analyzeLoop();

public:
    explicit MASFunction(Function &func) : func(func) {
        analyzeLoop();
    }

    auto &getRawFunction() {
        return func;
    }

    auto &getIndVarSet() {
        return indVars;
    }

    auto &getMeta() {
        return meta;
    }
};

class MASModule {
    vector<MASFunction *> functions;

public:
    void analyze(Module &m);

    vector<MASFunction *> &getFunctions() {
        return functions;
    }
};
