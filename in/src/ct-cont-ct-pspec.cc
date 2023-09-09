// ct-cont-ct-pspec.cc
// Demonstrate partial specialization of a class template inside another class template.

template <class T>
struct Outer {
  template <class U>
  struct Inner;
};

template <class T>
template <class V>
struct Outer<T>::Inner<V*> {
  T t;
  V *u;
};

Outer<int>::Inner<float*> i;

// EOF
