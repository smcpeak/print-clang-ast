// typedef-struct-field.cc
// Typedef a `struct` then use that as a field.

struct A;

typedef A A_typedef;

// Define `A` *after* creating the `typedef`.
struct A {};

struct B {
  A_typedef a;
};

// EOF
