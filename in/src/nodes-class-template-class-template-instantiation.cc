// nodes-class-template-class-template-instantiation.cc
// Instantiate a class template inside a class template.

template <class T>
struct Outer {
  template <class U>
  struct Inner {
    T t;
    U u;
  };
};

Outer<int>::Inner<float> i;

// EOF
