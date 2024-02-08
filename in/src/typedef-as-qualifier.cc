// typedef-as-qualifier.cc
// Use a typedef as a qualifier.

// The enclosing namespace is not relevant to the essential problem of
// using a typedef as a qualifier, but the premise is we want fully
// qualified names in order to move them outside the namespace
// declaration, which is why we cannot just use the Python 'spelling'
// method, since that yields "B::Inner" rather than "ns::B::Inner".
namespace ns {

  struct A {
    struct Inner {};
  };
  typedef A B;

  // Here, we use the typedef 'B' as a qualifier when describing the
  // return type.  The question then is how can we retrieve this qualifier
  // to print it (rather than 'A') after parsing, as asked in the second
  // revision of this SO question:
  //
  //   https://stackoverflow.com/questions/77941127/how-can-i-get-the-fully-qualified-names-of-return-types-and-argument-types-using
  //
  // The goal here is to construct the string "ns::B::Inner" to
  // represent the return type.
  //
  B::Inner f();

}

// EOF
