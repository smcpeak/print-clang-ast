// ct-cont-oc-emspec.cc
// Explicit member specialization of an ordinary class inside a class template.

template <class T>
struct Outer {
  struct Inner;
};

template <>
struct Outer<int>::Inner {
  int t;
  float u;
};

// EOF
