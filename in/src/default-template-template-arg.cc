// default-template-template-arg.cc
// Template template argument with a default.

template <typename T>
struct A;

template <template <typename T> class U = A>
struct B;

// EOF
