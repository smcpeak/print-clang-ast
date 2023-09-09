// has-defaulted-func-info.cc
// An example where FunctionDecl::HasDefaultedFunctionInfo is true.

struct S;

// These operators will be in the lookup set of the
// DefaultedFunctionInfo associated with the member operator below.
bool operator==(S const &, S const &);
bool operator==(S, S);

struct S {
  // This operator has DefaultedFunctionInfo.
  bool operator==(S const &) const = default;
};

// EOF
