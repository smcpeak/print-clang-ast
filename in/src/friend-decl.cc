// friend-decl.cc
// Examples of 'friend' declarations.

struct S1 {
  int m();
};

struct S2 {
  friend struct S1;
  friend int S1::m();
  friend int newGlobalFunc();
};

// EOF
