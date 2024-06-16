// new-dependent-type.cc
// Use `new` to create something with dependent type.

template <typename T>
void f()
{
  new T;
}

// EOF
