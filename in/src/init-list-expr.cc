// init-list-expr.cc
// Demo of InitListExpr.

struct S {
  int a;
  int b;
};

S s = {1,2};

S s2 = { .b = 3 };

// EOF
