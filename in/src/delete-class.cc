// delete-class.cc
// Use `delete` on a class type.

struct S {};

void f(S *s)
{
  delete s;
}

// EOF
