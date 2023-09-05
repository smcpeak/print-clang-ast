// nodes-function-template-explicit-specialization.cc
// Function template explicit specialization.

template <class T>
T identity(T t);

template <>
int identity(int t)
{
  return t;
}

// EOF
