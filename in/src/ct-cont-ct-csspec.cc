// ct-cont-ct-csspec.cc
// Specialize the inner CT inside the body of the outer CT.

template <class T>
struct Outer {
  template <class U>
  struct Inner;

  template <>
  struct Inner<float> {
    T t;
    float u;
  };
};

Outer<int>::Inner<float> i;

// EOF
