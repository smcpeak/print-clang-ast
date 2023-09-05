// nodes-class-template-only.cc
// ClassTemplateDecl.

template <class T>
struct S {
  T data;
  S *ptr1;
  S<T> *ptr2;
};

// EOF
