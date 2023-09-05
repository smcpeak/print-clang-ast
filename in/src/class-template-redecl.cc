// nodes-class-template-redecl.cc
// Class template, then redeclared.

template <class T>
struct S;

template <class T>
struct S {
  T data;
};

// EOF
