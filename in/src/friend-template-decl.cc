// friend-template-decl.cc
// FriendTemplateDecl?

template <typename T>
class MyVector;

template <typename T>
class Foo {
  template <typename U>
  class Nested;
};

template <typename T>
class A {
  // not a friend template
  friend class MyVector<T>;

  // not a friend template
  template <typename U>
  friend class B;

  // friend template
  template <typename U>
  friend class Foo<T>::Nested;
};

// EOF
