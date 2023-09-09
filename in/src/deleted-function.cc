// deleted-function.cc
// AST nodes test for a function that is deleted.

struct S {
  void delmethod() = delete;
};

// Due to a clang bug, the end location is wrong here.
void delfunc() = delete;

// EOF
