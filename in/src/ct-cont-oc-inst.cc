// ct-cont-oc-inst.cc
// Instantiate a class template that contains an ordinary class.

template <class T>
struct Outer {
  struct Inner {
    T t;
    float u;
  };
};

Outer<int>::Inner i;

// EOF
