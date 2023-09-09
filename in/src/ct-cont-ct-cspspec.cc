// ct-cont-ct-cspspec.cc
// Partially specialize the inner CT inside the body of the outer CT.

template <class T>
struct Outer {
  template <class U>
  struct Inner;

  template <class V>
  struct Inner<V*> {
    T t;
    V *u;
  };
};

Outer<int>::Inner<float*> i;

// EOF
