// var-template.cc
// Variable template.

template <class T>
constexpr unsigned VarTemplate = sizeof(T) * 8;

struct S {
  int x;
};

unsigned f()
{
  return VarTemplate<S>;
}

// EOF
