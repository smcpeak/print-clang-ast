// decl-implicit.cc
// Code for decl-implicit module.

#include "decl-implicit-private.h"               // private decls for this module

#include "trace.h"                               // INIT_TRACE, TRACE1

#include "clang/Sema/Sema.h"                     // Sema::ForceDeclarationOfImplicitMembers

using clang::dyn_cast;


DeclareImplicitThings::DeclareImplicitThings(
  clang::ASTUnit *astUnit,
  bool defineAlso)
  :
    ClangUtil(astUnit->getASTContext()),
    clang::RecursiveASTVisitor<DeclareImplicitThings>(),
    m_astUnit(astUnit),
    m_defineAlso(defineAlso)
{}


DeclareImplicitThings::~DeclareImplicitThings()
{}


clang::Sema &DeclareImplicitThings::getSema()
{
  return m_astUnit->getSema();
}


bool DeclareImplicitThings::VisitCXXRecordDecl(clang::CXXRecordDecl *decl)
{
  INIT_TRACE("DeclareImplicitThings::VisitCXXRecordDecl");

  // Only force declarations when looking at the definition.  Otherwise,
  // the new members can end up with source locations derived from a
  // forward declaration, which is wrong from a dependency point of
  // view.
  if (decl->isThisDeclarationADefinition()) {
    TRACE1("class: " << namedDeclAtLocStr(decl));
    getSema().ForceDeclarationOfImplicitMembers(decl);
  }

  return true;
}


bool DeclareImplicitThings::VisitCXXMethodDecl(
  clang::CXXMethodDecl *decl)
{
  if (!m_defineAlso) {
    return true;
  }

  // Check the the common invariants for a method that can be implicitly
  // defined.
  if (!( decl->isDefaulted() &&
         !decl->doesThisDeclarationHaveABody() &&
         !decl->isDeleted() )) {
    return true;
  }

  INIT_TRACE("DeclareImplicitThings::VisitCXXMethodDecl");
  TRACE1("method: " << namedDeclAtLocStr(decl));

  // Location to use as the 'CurrentLocation' for the calls to
  // DefineImplicitXXX.
  clang::SourceLocation loc = decl->getLocation();

  if (auto ctor = dyn_cast<clang::CXXConstructorDecl>(decl)) {
    if (ctor->isDefaultConstructor()) {
      getSema().DefineImplicitDefaultConstructor(loc, ctor);
    }

    else if (ctor->isCopyConstructor()) {
      getSema().DefineImplicitCopyConstructor(loc, ctor);
    }

    else if (ctor->isMoveConstructor()) {
      getSema().DefineImplicitMoveConstructor(loc, ctor);
    }
  }

  else if (auto dtor = dyn_cast<clang::CXXDestructorDecl>(decl)) {
    getSema().DefineImplicitDestructor(loc, dtor);
  }

  // Disable the remainder to work around
  // https://github.com/llvm/llvm-project/issues/67862.
  else if (true) {
    return true;
  }

  else if (decl->isCopyAssignmentOperator()) {
    getSema().DefineImplicitCopyAssignment(loc, decl);
  }

  else if (decl->isMoveAssignmentOperator()) {
    getSema().DefineImplicitMoveAssignment(loc, decl);
  }

  // TODO: Sema also has LambdaToFunctionPointerConversion and
  // LambdaToBlockPointerConversion.  I don't know what these are, so
  // for now am not doing anything with them.

  return true;
}


void declareImplicitThings(
  clang::ASTUnit *astUnit,
  bool defineAlso)
{
  DeclareImplicitThings dit(astUnit, defineAlso);
  dit.TraverseDecl(astUnit->getASTContext().getTranslationUnitDecl());
}


// EOF
