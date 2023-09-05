// nodes-method-template-of-concrete-class.cc
// Method template of a non-template (concrete) class.

struct S {
  template <class T>
  T identity(T t)
  {
    return t;
  }
};

int caller(S &s, int x)
{
  return s.identity(x);
}

// EOF
