// ct-inner-struct.cc
// A class template with an inner (non-template) struct.

template <class T>
struct S {
  T data;

  struct Inner {};
};

S<int> s;
S<int>::Inner i;

// EOF
