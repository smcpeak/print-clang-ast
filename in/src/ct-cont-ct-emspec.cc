// ct-cont-ct-emspec.cc
// Explicitly specialize a member class template inside a class template.

template <class T>
struct Outer {
  template <class U>
  struct Inner;
};

template <>
template <class U>
struct Outer<int>::Inner {
  int t;
  U u;
};

Outer<int>::Inner<float> i;

// EOF
