#ifndef SCOMMON_H
#define SCOMMON_H

static void printDebugLocIfPosibble(Value *v) {
  if (auto inst = dyn_cast<Instruction>(v)) {
    auto dl = inst->getDebugLoc();
    errs() << "file: " << inst->getModule()->getName().str() << ",   "
           << inst->getFunction()->getName().str();
    if (dl) {
      errs() << ",    row: " << dl.getLine() << ",  col: " << dl.getCol();
    }
    errs() << "\n";
  }
}

static string getStringFromValue(const Value *v) {
  string str;
  raw_string_ostream rso(str);
  v->print(rso);
  return str;
}

static string num2string(int i) {
  ostringstream oss;
  oss << i;
  return oss.str();
}

#endif
