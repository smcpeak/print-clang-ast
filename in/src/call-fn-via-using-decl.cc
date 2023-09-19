// call-fn-via-using-decl.cc
// Call a function via an intermediate 'using' declaration.

enum E {};

namespace NS1 {
  int fn(E, E);
}

namespace NS2 {
  using NS1::fn;

  int caller(E e)
  {
    return fn(e, e);
  }
}

// EOF
