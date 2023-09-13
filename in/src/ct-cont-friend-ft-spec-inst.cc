// ct-cont-friend-ft-spec-inst.cc
// Class template contains friend function template specialization, and
// we instantiate the template.

template <class T>
T identity(T t);

template <class T>
class A {
  friend T identity<T>(T t);
  T m_t;
};

template <class T>
T identity(T t)
{
  A<T> a;
  a.m_t = t;
  return a.m_t;
}

int caller(int x)
{
  return identity(x);
}

// EOF
