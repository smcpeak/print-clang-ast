// nodes-function-template.cc
// Simple example of a function template.

template <class T>
T identity(T t)
{
  return t;
}

int caller(int x)
{
  return identity(x);
}

// EOF
