struct munger_struct {
  int f1;
  int f2;
};

void munge(struct munger_struct *P) {
  P[0].f1 = P[1].f1 + P[2].f2;
}
