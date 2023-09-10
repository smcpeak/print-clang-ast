// oc-cont-ft-defn.cc
// Method template of a non-template (concrete) class.

struct S {
  template <class T>
  T identity(T t)
  {
    return t;
  }
};

// EOF
