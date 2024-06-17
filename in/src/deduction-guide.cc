// deduction-guide.cc
// Simple deduction guide example.

// This comes from the comment above `CXXDeductionGuideDecl` in
// `clang/AST/DeclCXX.h`.

template <typename T>
struct A {
  A();
  A(T);
};

A() -> A<int>;

void f()
{
  // Uses the explicit deduction guide to deduce `A<int>`.
  A a_int;

  // Uses the implicit deduction guide associated with the second ctor
  // to deduce `A<double>`.
  A a_double(3.14);
}

// EOF
