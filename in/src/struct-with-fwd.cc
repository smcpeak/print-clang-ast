// struct-with-fwd.cc
// Struct with forward declaration.

struct Foo;

// The point of this test is I want the implicit member declarations to
// have source locations associated with this definition, not the
// forward above.
struct Foo {
  int x;
};

// EOF
