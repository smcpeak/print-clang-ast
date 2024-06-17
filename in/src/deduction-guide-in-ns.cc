// deduction-guide-in-ns.cc
// Deduction guide that is inside a namespace.

namespace NS {
  template <typename T>
  struct S {
    typedef int size_type;

    // In conjunction with the explicit deduction guide below, this
    // causes an implicit guide to be created.  But the implicit guide
    // is placed into the namespace (not the class), along with the
    // explicit guide.  Consequently, the user-written `size_type` is
    // not in scope, so Clang synthesizes a copy of it, and sets the
    // implicit deduction guide as its lexical (and semantic) parent.
    // That confuses my analysis currently.
    S(size_type pos);
  };

  // Note: `S<char>` is not instantiated per se anywhere, it is only
  // named here in the deduction guide.  This causes Clang to use the
  // `TSK_Undeclared` specialization kind for it.  If a normal
  // instantiation or specialization happened, (as in
  // `deduction-guide.cc`), then it would become one of the other kinds
  // as appropriate.
  S(int) -> S<char>;
}

// EOF
