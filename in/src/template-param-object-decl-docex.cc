// template-param-object-decl-docex.cc
// Example of TemplateParamObjectDecl from the API documentation.

struct A { int x, y; };
template<A> struct S {};        // smcpeak: Added "{}" to fix error.
S<A{1, 2}> s1;
S<A{1, 2}> s2; // same type, argument is same TemplateParamObjectDecl.

// EOF
