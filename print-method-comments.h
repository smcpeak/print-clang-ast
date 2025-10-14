// print-method-comments.h
// Visitor to find and print comments on method declarations.

#ifndef PRINT_CLANG_AST_PRINT_METHOD_COMMENTS_H
#define PRINT_CLANG_AST_PRINT_METHOD_COMMENTS_H

#include "clang/AST/ASTContext.h"      // clang::ASTContext

#include <iosfwd>                      // std::ostream


// Find and print comments on method declarations.
void printMethodComments(
  std::ostream &os,
  clang::ASTContext &astContext);


#endif // PRINT_CLANG_AST_PRINT_METHOD_COMMENTS_H
