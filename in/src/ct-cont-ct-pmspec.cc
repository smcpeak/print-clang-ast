// ct-cont-ct-pmspec.cc
// Partially specialize a member class template inside a class template.

template <class T>
struct Outer {
  template <class U>
  struct Inner;
};

template <>
template <class V>
struct Outer<int>::Inner<V*> {
  int t;
  V *u;
};

Outer<int>::Inner<float*> i;

// NOTE: This test does not work in Clang-16 or earlier because of
// https://github.com/llvm/llvm-project/issues/60778

// EOF
