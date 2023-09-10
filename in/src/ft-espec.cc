// ft-espec.cc
// Function template explicit specialization.

template <class T>
T identity(T t);

template <>
int identity(int t)
{
  return t;
}

// EOF
