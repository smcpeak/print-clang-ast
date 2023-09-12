// decl-implicit.cc
// Code for decl-implicit module.

#include "decl-implicit-private.h"               // private decls for this module

#include "clang/Sema/Sema.h"                     // Sema::ForceDeclarationOfImplicitMembers


DeclareImplicitThings::DeclareImplicitThings(clang::ASTUnit *astUnit)
  : ClangUtil(astUnit->getASTContext()),
    clang::RecursiveASTVisitor<DeclareImplicitThings>(),
    m_astUnit(astUnit)
{}


DeclareImplicitThings::~DeclareImplicitThings()
{}


bool DeclareImplicitThings::VisitCXXRecordDecl(clang::CXXRecordDecl *decl)
{
  m_astUnit->getSema().ForceDeclarationOfImplicitMembers(decl);
  return true;
}


void declareImplicitThings(
  clang::ASTUnit *astUnit)
{
  DeclareImplicitThings dit(astUnit);
  dit.TraverseDecl(astUnit->getASTContext().getTranslationUnitDecl());
}


// EOF
