// ct-pspec-notdef.cc
// Demonstrate partial specialization when there is no definition.

template <class T>
struct S;

template <class U>
struct S<U*>;

extern S<int*> s;

// EOF
