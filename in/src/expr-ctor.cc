// expr-ctor.cc
// Expression that is a ctor call.

struct S {
  int f();
};

int caller()
{
  return S().f();
}

// EOF
