// expr-lambda.cc
// Lambda expression.

int f(int z)
{
  auto lambdaNoCapture = [](int x, int y) -> int {
    return x+y;
  };
  auto lambdaWithCapture = [&](int x, int y) -> int {
    return x+y+z;
  };

  return lambdaNoCapture(3, 4) +
         lambdaWithCapture(5, 6);
}

// EOF
