// call-static-method.cc
// Call a static method.

struct A {
  static void foo();
};

void caller()
{
  A::foo();
}

// EOF
