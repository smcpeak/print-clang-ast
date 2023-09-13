// ft-cont-lambda-inst.cc
// Function template contains lambda.

template <class T>
T identity(T x)
{
  auto f =
    [](T z) -> T {
      return z;
    };

  return f(x);
}

int caller(int y)
{
  return identity(y);
}

// EOF
