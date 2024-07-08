// clang-util-ast-visitor.cc
// Code for `clang-util-ast-visitor` module.

#include "clang-util-ast-visitor.h"    // this module


// ------------------------ ClangUtilASTVisitor ------------------------
ClangUtilASTVisitor::ClangUtilASTVisitor(clang::ASTContext &context)
  : ClangUtil(context),
    ClangASTVisitor()
{}


void ClangUtilASTVisitor::scanTU()
{
  ClangASTVisitor::scanTU(getASTContext());
}


// ----------------------- ClangUtilASTVisitorNC -----------------------
ClangUtilASTVisitorNC::ClangUtilASTVisitorNC(clang::ASTContext &context)
  : ClangUtil(context),
    ClangASTVisitorNC()
{}


void ClangUtilASTVisitorNC::scanTU()
{
  ClangASTVisitorNC::scanTU(getASTContext());
}


// EOF
