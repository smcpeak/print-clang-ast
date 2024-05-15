// default-template-non-type-arg.cc
// Template with default non-type argument.

int const c = 3;

template <int n = c>
struct S;

// EOF
