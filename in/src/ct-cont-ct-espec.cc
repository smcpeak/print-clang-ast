// ct-cont-ct-espec.cc
// Explicitly specialize a class template inside a class template.

template <class T>
struct Outer {
  template <class U>
  struct Inner;
};

template <>
template <>
struct Outer<int>::Inner<float> {
  int t;
  float u;
};

// EOF
