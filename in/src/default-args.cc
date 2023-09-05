// nodes-default-args.cc
// Simple test of node printing with default arguments.

int f(int x = 2)
{
  return x;
}

int g(int x)
{
  return x;
}

int h(int x = 3);
int h(int x)
{
  return x;
}

// EOF
