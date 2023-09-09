// ct-pspec.cc
// Demonstrate partial specialization.

template <class T>
struct S;

template <class U>
struct S<U*>
{
  U *data;
  S *ptr1;
  S<U*> *ptr2;
};

S<int*> s;

// EOF
