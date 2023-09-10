// ct-cont-of-inst.cc
// Class template with a method.

template <class T>
struct S {
  T identity(T t)
  {
    return t;
  }
};

int call(S<int> &s, int x)
{
  return s.identity(x);
}

// EOF
