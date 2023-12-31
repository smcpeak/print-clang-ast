// ct-cont-ft-defn.cc
// Class template with a method template.

template <class T>
struct S {
  template <class U>
  unsigned sum(T t, U u)
  {
    return t + u;
  }
};

// EOF
