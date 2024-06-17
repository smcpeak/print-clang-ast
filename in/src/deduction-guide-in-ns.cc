// deduction-guide-in-ns.cc
// Deduction guide that is inside a namespace.

namespace NS {
  template <typename T>
  struct S {
    typedef int size_type;

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
