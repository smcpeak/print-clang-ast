// call-op-via-using-decl.cc
// Call an operator via an intermediate 'using' declaration.

enum E {};

namespace NS1 {
  int operator& (E, E);
}

namespace NS2 {
  using NS1::operator&;

  int caller(E e)
  {
    return e & e;
    //return operator&(e, e);
  }
}

// EOF
