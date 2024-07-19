// ct-redecl-inst.cc
// Class template, then redeclared, then instantiated.

template <class T>
struct S;

template <class T>
struct S {
  T data;
};

S<int> s;

// EOF
