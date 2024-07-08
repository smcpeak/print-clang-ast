// clang-util-ast-visitor.cc
// Code for `clang-util-ast-visitor` module.

#include "clang-util-ast-visitor.h"    // this module


ClangUtilASTVisitor::ClangUtilASTVisitor(clang::ASTContext &context)
  : ClangUtil(context),
    ClangASTVisitor()
{}


void ClangUtilASTVisitor::scanTU()
{
  ClangASTVisitor::scanTU(getASTContext());
}


// EOF
