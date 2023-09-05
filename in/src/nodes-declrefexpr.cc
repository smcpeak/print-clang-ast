// nodes-declrefexpr.cc
// Print nodes test exercising some DeclRefExpr cases.

struct S {
  operator int* ();
};

int f(int x, S *s)
{
  // I'm not sure how to get a DRE that refers to a ctor or dtor.
  //&S::~S;

  // conversion operator
  int* (S::*ptm)() = &S::operator int*;
  (s->*ptm)();

  // Another case is a literal operator but I don't know how to do that.

  // Plain DRE for a variable.
  return x;
}

// EOF
