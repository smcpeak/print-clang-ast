// ct-cont-ft-espec.cc
// Demonstrate explicitly specializing a method template that is
// declared inside a class template.

template <class T>
struct S {
  template <class U>
  unsigned sum(T t, U u);
};

template <>
template <>
unsigned S<int>::sum(int t, float u)
{
  return t + u;
}

// EOF
