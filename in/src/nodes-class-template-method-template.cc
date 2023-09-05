// nodes-class-template-method-template.cc
// Class template with a method template.

template <class T>
struct S {
  template <class U>
  unsigned sum(T t, U u)
  {
    return t + u;
  }
};

unsigned caller(S<int> &s)
{
  return s.sum<float>(1, 2.0f);
}

// EOF
