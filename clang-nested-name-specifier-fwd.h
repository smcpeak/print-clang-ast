// clang-nested-name-specifier-fwd.h
// Forwards for clang/AST/NestedNameSpecifier.h.

#ifndef CLANG_NESTED_NAME_SPECIFIER_FWD_H
#define CLANG_NESTED_NAME_SPECIFIER_FWD_H

namespace clang {
  class NestedNameSpecifier;
  class NestedNameSpecifierLoc;

  // It's not clear whether this one should be treated as part of the
  // public API.
  class NestedNameSpecifierLocBuilder;
}

#endif // CLANG_NESTED_NAME_SPECIFIER_FWD_H
