// ct-inst.cc
// Define a class template and implicitly instantiate it.

template <class T>
struct S {
  T data;
  S *ptr1;
  S<T> *ptr2;
};

S<int> s;

// EOF
