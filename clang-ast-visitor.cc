// clang-ast-visitor.cc
// Code for clang-ast-visitor.h.

#include "clang-ast-visitor.h"                   // this module

// this dir
#include "clang-util.h"                          // assert_dyn_cast

// clang
#include "clang/AST/DeclFriend.h"                // clang::{FriendDecl, ...}

using clang::dyn_cast;


void ClangASTVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  if (auto dd = dyn_cast<clang::DeclaratorDecl>(decl)) {
    clang::TypeSourceInfo const *tsi = dd->getTypeSourceInfo();
    assert(tsi);
    visitTypeLoc(VTC_DECLARATOR_TYPE, tsi->getTypeLoc());

    if (clang::Expr const *trailingRequires =
          dd->getTrailingRequiresClause()) {
      visitStmt(VSC_DECLARATOR_TRAILING_REQUIRES, trailingRequires);
    }

    else if (auto vd = dyn_cast<clang::VarDecl>(decl)) {
      if (clang::Expr const *init = vd->getInit()) {
        visitStmt(VSC_VAR_DECL_INIT, init);
      }
    }

    else if (auto fd = dyn_cast<clang::FunctionDecl>(decl)) {
      visitFunctionParameters(fd);

      if (auto ccd = dyn_cast<clang::CXXConstructorDecl>(decl)) {
        visitCtorInitializers(ccd);
      }

      // For CXXConversionDecl, the TypeLoc is in the return type of the
      // function type, which is visited above as the declarator type.

      if (fd->doesThisDeclarationHaveABody()) {
        clang::Stmt const *body = fd->getBody();
        assert(body);
        visitStmt(VSC_FUNCTION_BODY, body);
      }
    }

    else if (auto fd = dyn_cast<clang::FieldDecl>(decl)) {
      if (clang::Expr const *width = fd->getBitWidth()) {
        visitStmt(VSC_FIELD_BIT_WIDTH, width);
      }

      if (clang::Expr const *init = fd->getInClassInitializer()) {
        visitStmt(VSC_FIELD_INIT, init);
      }
    }

    else if (auto ecd = dyn_cast<clang::EnumConstantDecl>(decl)) {
      if (clang::Expr const *init = ecd->getInitExpr()) {
        visitStmt(VSC_ENUM_CONSTANT_INIT, init);
      }
    }
  }

  else if (auto tnd = dyn_cast<clang::TypedefNameDecl>(decl)) {
    clang::TypeSourceInfo const *tsi = tnd->getTypeSourceInfo();
    assert(tsi);
    visitTypeLoc(VTC_TYPEDEF_NAME_TYPE, tsi->getTypeLoc());

    // TypeAliasDecl carries a 'Template' pointer, but I think that
    // points to a parent AST node, and consequently we should not visit
    // it.
  }

  else if (auto ed = dyn_cast<clang::EnumDecl>(decl)) {
    if (clang::TypeSourceInfo *tsi = ed->getIntegerTypeSourceInfo()) {
      visitTypeLoc(VTC_ENUM_UNDERLYING, tsi->getTypeLoc());
    }

    visitNonFunctionDeclContext(assert_dyn_cast(clang::DeclContext, ed));
  }

  else if (auto fsad = dyn_cast<clang::FileScopeAsmDecl>(decl)) {
    visitStmt(VSC_FILE_SCOPE_ASM_STRING, fsad->getAsmString());
  }

  // BlockDecl is ObjC I think.

  // TODO: CapturedDecl

  else if (auto crd = dyn_cast<clang::CXXRecordDecl>(decl)) {
    visitCXXRecordBases(crd);
    visitNonFunctionDeclContext(assert_dyn_cast(clang::DeclContext, ed));
  }

  // TODO: CXXDeductionGuideDecl?

  // TODO: BindingDecl, DecompositionDecl

  else if (auto td = dyn_cast<clang::TemplateDecl>(decl)) {
    visitTemplateParameterList(td->getTemplateParameters());
    visitDecl(VDC_TEMPLATE_TEMPLATED, td);

    // TODO: The various TemplateDecl subclasses.

    if (auto ctd = dyn_cast<clang::ClassTemplateDecl>(decl)) {
      visitClassTemplateInstantiations(ctd);
    }

    else if (auto ftd = dyn_cast<clang::FunctionTemplateDecl>(decl)) {
      visitFunctionTemplateInstantiations(ftd);
    }
  }

  else {
    switch (decl->getKind()) {
      #define HANDLE_KIND(kind, varName, ...)                      \
        case clang::Decl::kind: {                                  \
          auto varName = assert_dyn_cast(clang::kind##Decl, decl); \
          __VA_ARGS__;                                             \
          break;                                                   \
        }

      HANDLE_KIND(Friend, fd,
        if (auto inner = fd->getFriendDecl()) {
          visitDecl(VDC_FRIEND_FRIEND, inner);
        }
      )

      HANDLE_KIND(FriendTemplate, ftd,
        if (auto inner = ftd->getFriendDecl()) {
          visitDecl(VDC_FRIEND_TEMPLATE_FRIEND, inner);
        }
      )

      // DeclContexts other than FunctionDecl.
      case clang::Decl::Export:
      case clang::Decl::ExternCContext:
      case clang::Decl::LinkageSpec:
      case clang::Decl::Namespace:
      case clang::Decl::Record:
      case clang::Decl::RequiresExprBody:
      case clang::Decl::ClassTemplateSpecialization:
      case clang::Decl::ClassTemplatePartialSpecialization:
      case clang::Decl::TranslationUnit: {
        auto dc = assert_dyn_cast(clang::DeclContext, decl);
        visitNonFunctionDeclContext(dc);
        break;
      }

      // Ignore all others.
      default:
        break;

      #undef HANDLE_KIND
    }
  }
}


void ClangASTVisitor::visitStmt(VisitStmtContext context,
                                clang::Stmt const *stmt)
{
  // TODO
}


void ClangASTVisitor::visitTypeLoc(VisitTypeContext context,
                                   clang::TypeLoc typeLoc)
{
  // TODO
}


void ClangASTVisitor::visitNonFunctionDeclContext(
  clang::DeclContext const *dc)
{
  for (clang::Decl const *d : dc->decls()) {
    visitDecl(VDC_DECL_CONTEXT_DECL, d);
  }
}


void ClangASTVisitor::visitFunctionTemplateInstantiations(
  clang::FunctionTemplateDecl const *ftd)
{
  for (clang::FunctionDecl const *spec : ftd->specializations()) {
    if (spec->isTemplateInstantiation()) {
      visitDecl(VDC_FUNCTION_TEMPLATE_INSTANTIATION, spec);
    }
  }
}


void ClangASTVisitor::visitClassTemplateInstantiations(
  clang::ClassTemplateDecl const *ctd)
{
  for (clang::ClassTemplateSpecializationDecl const *spec :
          ctd->specializations()) {
    if (clang::isTemplateInstantiation(spec->getSpecializationKind())) {
      visitDecl(VDC_CLASS_TEMPLATE_INSTANTIATION, spec);
    }
  }
}


void ClangASTVisitor::visitFunctionParameters(
  clang::FunctionDecl const *fd)
{
  for (clang::ParmVarDecl const *param : fd->parameters()) {
    visitDecl(VDC_FUNCTION_PARAMETER, param);
  }
}


void ClangASTVisitor::visitCXXRecordBases(
  clang::CXXRecordDecl const *crd)
{
  for (clang::CXXBaseSpecifier const &base : crd->bases()) {
    visitBaseSpecifier(base);
  }
}


void ClangASTVisitor::visitBaseSpecifier(
  clang::CXXBaseSpecifier const &base)
{
  clang::TypeSourceInfo const *tsi = base.getTypeSourceInfo();
  assert(tsi);
  visitTypeLoc(VTC_CLASS_BASE, tsi->getTypeLoc());
}


void ClangASTVisitor::visitCtorInitializers(
  clang::CXXConstructorDecl const *ccd)
{
  for (clang::CXXCtorInitializer const *init : ccd->inits()) {
    visitCtorInitializer(init);
  }
}


void ClangASTVisitor::visitCtorInitializer(
  clang::CXXCtorInitializer const *init)
{
  if (clang::TypeSourceInfo const *tsi = init->getTypeSourceInfo()) {
    visitTypeLoc(VTC_CTOR_INIT, tsi->getTypeLoc());
  }
  else {
    // The initializer initializes a member (rather than a base, or
    // indicating a delegation), which we do not visit.
  }
}


void ClangASTVisitor::visitTemplateParameterList(
  clang::TemplateParameterList const *tparams)
{
  for (clang::NamedDecl const *param : *tparams) {
    visitDecl(VDC_TEMPLATE_PARAMETER, param);
  }

  if (clang::Expr const *expr = tparams->getRequiresClause()) {
    visitStmt(VSC_TEMPLATE_REQUIRES_CLAUSE, expr);
  }

  // TODO: Associated constraints?
}


void ClangASTVisitor::visitTemplateArgumentList(
  clang::TemplateArgumentList const *targs)
{
  for (clang::TemplateArgument const &arg : targs->asArray()) {
    visitTemplateArgument(arg);
  }
}


void ClangASTVisitor::visitTemplateArgument(
  clang::TemplateArgument const &arg)
{
  // TODO
}


// EOF
