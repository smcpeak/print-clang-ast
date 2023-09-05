// nodes-class-template-explicit-specialization.cc
// Class template explicit specialization.

template <class T>
struct S;

template <>
struct S<int>
{
  int data;
  S *ptr1;
  S<int> *ptr2;
};

// EOF
