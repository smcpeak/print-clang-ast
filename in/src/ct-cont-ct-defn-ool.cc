// ct-cont-ct-defn-ool.cc
// A class template contains a class template, where the inner one
// is defined out of line.

template <class T>
struct Outer {
  template <class U>
  struct Inner;
};

template <class T>
template <class U>
struct Outer<T>::Inner {
  T t;
  U u;
};

Outer<int>::Inner<float> i;

// EOF
