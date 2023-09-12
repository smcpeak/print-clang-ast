// ft-cont-oc-inst.cc
// Function template contains ordinary class.

template <class T>
T identity(T x)
{
  struct S {
    T m_t;

    S(T t)
      : m_t(t)
    {}
  };

  S s(x);
  return s.m_t;
}

int caller(int y)
{
  return identity(y);
}

// EOF
