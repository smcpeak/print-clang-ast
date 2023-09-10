// ct-cont-ft-csspec.cc
// "Partial" specialization of a method template inside a class template.

template <class T>
struct S {
  template <class U>
  int add(T t, U u);

  template <>              // ClassScopeFunctionSpecializationDecl
  int add(T t, float u)
  {
    return t + u;
  }
};

int caller(S<int> &s, int i, float f)
{
  return s.add(i, f);
}

// EOF
