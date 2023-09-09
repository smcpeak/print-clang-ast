// nested-function.cc
// Printing test for a nested functions.

int f(int x)
{
  struct S {
    static int g(int x)
    {
      return x;
    }
  };

  auto h = [](int x) { return x; };

  return S::g(x);
}

// EOF
