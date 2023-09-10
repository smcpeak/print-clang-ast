// ct-cont-ft-emspec.cc
// Class template contains function template, which then has an
// explicit member specialization.

template <class T>
struct S {
  template <class U>
  unsigned sum(T t, U u);
};

template <>
template <class U>
unsigned S<int>::sum(int t, U u)
{
  return t + u;
}

int caller(S<int> &s, int i, float f)
{
  return s.sum(i, f);
}

// EOF
