// decl-implicit.h
// AST pass to declare implicit things.

#ifndef DECL_IMPLICIT_H
#define DECL_IMPLICIT_H

#include "clang/Frontend/ASTUnit.h"              // clang::ASTUnit

// Make a pass over the AST, creating declarations for implicit things.
void declareImplicitThings(
  clang::ASTUnit *astUnit);

#endif // DECL_IMPLICIT_H
