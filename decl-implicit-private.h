// decl-implicit-private.h
// Private declarations for decl-implicit module.

#ifndef DECL_IMPLICIT_PRIVATE_H
#define DECL_IMPLICIT_PRIVATE_H

#include "decl-implicit.h"                       // public decls for this module

#include "clang-util.h"                          // ClangUtil

#include "clang/AST/RecursiveASTVisitor.h"       // clang::RecursiveASTVisitor
#include "clang/Sema/Sema.h"                     // clang::Sema


// Force declarations of all implicit things we can.
class DeclareImplicitThings :
  public ClangUtil,
  public clang::RecursiveASTVisitor<DeclareImplicitThings>
{
public:      // data
  // The ASTUnit we want to process.  Unlike 'ASTContext', 'ASTUnit'
  // provides access to the 'Sema' object.
  clang::ASTUnit *m_astUnit;

  // True to also provide definitions.
  bool m_defineAlso;

public:      // methods
  DeclareImplicitThings(clang::ASTUnit *astUnit, bool defineAlso);
  ~DeclareImplicitThings();

  // Get the 'Sema' object from 'm_astUnit'.
  clang::Sema &getSema();

  // Visit everything.
  bool shouldVisitTemplateInstantiations() const
    { return true; }
  bool shouldVisitImplicitCode() const
    { return true; }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);
  bool VisitCXXMethodDecl(clang::CXXMethodDecl *decl);
};


#endif // DECL_IMPLICIT_PRIVATE_H
