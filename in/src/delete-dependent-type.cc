// delete-dependent-type.cc
// Apply `delete` to a dependent type.

template <typename T>
void f(T *t)
{
  delete t;
}

// EOF
