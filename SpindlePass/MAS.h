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

class MASModule;
class MASFunction;

struct LoopIndVar {
    Value *initValue, *finalValue;
    ASTAbstractNode *delta;
};

class MASLoop {
    Loop &loop;
    MASFunction *parent;

public:
    SmallVector<LoopIndVar> indVars;  // computable loops' loop variables

    MASLoop(Loop &loop, MASFunction *func);

    auto isLoopInvariant(Value *v) const -> bool;
    auto analyze() -> bool;
};

struct InstrMetaInfo {
    bool isSTraceDependence = false;
    ASTAbstractNode *formula = nullptr;
    LoopIndVar *indVar = nullptr;
};

struct BBMetaInfo {
    bool needRecord = false, inMASLoop = false;
    MASLoop *loop = nullptr;
};

class MASFunction {
    LoopInfo LI;
    void analyzeLoop();

public:
    Function &func;
    MASModule *parent;
    set<Value *> indVars;
    map<Instruction *, InstrMetaInfo> instrMeta;
    map<BasicBlock *, BBMetaInfo> bbMeta;
    size_t num_loops;
    size_t num_computable_loops;

    MASFunction(Function &func, MASModule *module);
};

class MASModule {
public:
    Module *module;
    vector<MASFunction *> functions;
    size_t num_loops;
    size_t num_computable_loops;
    LLVMContext *context;

    explicit MASModule(Module &M);

    void analyze();
};
