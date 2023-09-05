// nodes-class-template-ovl-method-canttpt.cc
// Class template, with overloaded method, demonstrating need for
// canonical TemplateTypeParmType.

template <class T1, class U1>
struct S {
  int f(T1 t1, U1 u1);
  int f(U1 u1, T1 t1);
};

template <class T2, class U2>
int S<T2,U2>::f(T2 t2, U2 u2)
{
  return (int)sizeof(t2) - (int)sizeof(u2);
}

template <class T3, class U3>
int S<T3,U3>::f(U3 u3, T3 t3)
{
  return (int)sizeof(u3) - (int)sizeof(t3);
}

// EOF
