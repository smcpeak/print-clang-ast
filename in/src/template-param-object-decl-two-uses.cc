// template-param-object-decl-two-uses.cc
// Example of TemplateParamObjectDecl being used in two places.

struct A { int x, y; };

// This template is instantiated twice, but both instantiations use the
// same `TemplateParamObjectDecl` for the `A` argument.
template <A a, int i>
struct S {};

S<A{1, 2}, 3> s3;

S<A{1, 2}, 4> s4;

// EOF
