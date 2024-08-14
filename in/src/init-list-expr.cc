// init-list-expr.cc
// Demo of InitListExpr.

struct S {
  int a;
  int b;
};

S s = {1,2};

S s2 = { .b = 3 };

template <typename T>
void f()
{
  // Within an uninstantiated template, the InitListExpr has a semantic
  // form but no syntactic form.
  T t = {4,5};
}

// EOF
