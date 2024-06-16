// delete.cc
// Simple use of `delete` on a primitive.

void f(int *p1, int *p2)
{
  delete p1;
  delete[] p2;
}

// EOF
