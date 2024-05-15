// clang-expr-concepts-fwd.h
// Forwards for clang/AST/ExprConcepts.h.

#ifndef CLANG_EXPR_CONCEPTS_FWD_H
#define CLANG_EXPR_CONCEPTS_FWD_H

namespace clang {
  class ConceptSpecializationExpr;

  namespace concepts {
    class Requirement;
    class TypeRequirement;
    class ExprRequirement;
    class NestedRequirement;
  }

  // This is already in ASTFwd.h, but it shouldn't hurt here.
  class RequiresExpr;
}

#endif // CLANG_EXPR_CONCEPTS_FWD_H
