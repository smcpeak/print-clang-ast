// using-type-deduced-targs.cc
// Demonstrate a UsingType, and also deduction of class template argyments.

template <typename T>
struct A {
  A(T);                 // allows class template argument deduction
};

struct S {};

namespace NS1 {
  struct S;

  namespace NS2 {
    using ::S;

    void f()
    {
      S s;              // 'S' is a 'UsingType'
      A a(s);           // template arguments deduced
    }
  }
}

// EOF
