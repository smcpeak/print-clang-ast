// clang-util-ast-visitor.h
// Combination of `ClangUtil` and `ClangASTVisitor`.

#ifndef CLANG_UTIL_AST_VISITOR_H
#define CLANG_UTIL_AST_VISITOR_H

#include "clang-ast-context-fwd.h"     // clang::ASTContext
#include "clang-ast-visitor.h"         // ClangASTVisitor
#include "clang-util.h"                // ClangUtil


// AST visitor with the `ClangUtil` capabilities built-in.
class ClangUtilASTVisitor : public ClangUtil, public ClangASTVisitor {
public:      // methods
  ClangUtilASTVisitor(clang::ASTContext &context);

  // Slightly more convenient version of `scanTU` that does not need the
  // context passed explicitly.
  void scanTU();
};


#endif // CLANG_UTIL_AST_VISITOR_H
