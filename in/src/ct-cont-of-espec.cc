// ct-cont-of-espec.cc
// Explicitly specialize a non-template member of a class template.

template <class T>
struct S {
  T identity(T t);
};

template <>
int S<int>::identity(int t)
{
  return t;
}

// EOF
