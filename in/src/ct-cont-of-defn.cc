// ct-cont-of-defn.cc
// Class template with a method.

template <class T>
struct S {
  T identity(T t)
  {
    return t;
  }
};

// EOF
