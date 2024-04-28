// clang-ast-visitor.cc
// Code for clang-ast-visitor.h.

#include "clang-ast-visitor.h"                   // this module

// this dir
#include "clang-util.h"                          // assert_dyn_cast
#include "enum-util.h"                           // ENUM_TABLE_LOOKUP_CHECK_SIZE

// clang
#include "clang/AST/DeclFriend.h"                // clang::{FriendDecl, ...}

using clang::dyn_cast;


// Get the DeclContext aspect of 'decl'.
#define DECL_CONTEXT_OF(decl) assert_dyn_cast(clang::DeclContext, (decl))


char const *toString(VisitDeclContext vdc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitDeclContext,
    NUM_VISIT_DECL_CONTEXTS, vdc,

    VDC_NONE,

    VDC_ENUM_DECL,
    VDC_RECORD_DECL,
    VDC_EXPORT_DECL,
    VDC_EXTERN_C_DECL,
    VDC_LINKAGE_SPEC_DECL,
    VDC_NAMESPACE_DECL,
    VDC_REQUIRES_EXPR_BODY_DECL,
    VDC_TRANSLATION_UNIT_DECL,
    VDC_TEMPLATE_DECL,
    VDC_FRIEND_DECL,
    VDC_FRIEND_TEMPLATE_DECL,
    VDC_FUNCTION_TEMPLATE_INSTANTIATION,
    VDC_CLASS_TEMPLATE_INSTANTIATION,
    VDC_FUNCTION_DECL_PARAMETER,
    VDC_TEMPLATE_DECL_PARAMETER,

    VDC_FUNCTION_TYPE_PARAM,
  );

  return "unknown";
}


char const *toString(VisitStmtContext vsc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitStmtContext,
    NUM_VISIT_STMT_CONTEXTS, vsc,

    VSC_NONE,

    VSC_DECLARATOR_DECL_TRAILING_REQUIRES,
    VSC_VAR_DECL_INIT,
    VSC_FUNCTION_DECL_BODY,
    VSC_FIELD_DECL_BIT_WIDTH,
    VSC_FIELD_DECL_INIT,
    VSC_ENUM_CONSTANT_DECL,
    VSC_FILE_SCOPE_ASM_DECL_STRING,
    VSC_TEMPLATE_DECL_REQUIRES_CLAUSE,

    VSC_TYPE_OF_TYPE,
    VSC_DECLTYPE_TYPE,
    VSC_ARRAY_TYPE_SIZE,
  );

  return "unknown";
}


char const *toString(VisitTypeContext vtc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitTypeContext,
    NUM_VISIT_TYPE_CONTEXTS, vtc,

    VTC_NONE,

    VTC_DECLARATOR_DECL,
    VTC_TYPEDEF_NAME_DECL,
    VTC_ENUM_DECL_UNDERLYING,
    VTC_CXX_RECORD_DECL_BASE,
    VTC_CXX_CTOR_INITIALIZER,
    VTC_FRIEND_DECL,
    VTC_FRIEND_TEMPLATE_DECL,

    VTC_QUALIFIED_TYPE,
    VTC_ATTRIBUTED_TYPE,
    VTC_PAREN_TYPE,
    VTC_ADJUSTED_TYPE,
    VTC_POINTER_TYPE,
    VTC_BLOCK_POINTER_TYPE,
    VTC_MEMBER_POINTER_TYPE_CLASS,
    VTC_MEMBER_POINTER_TYPE_POINTEE,
    VTC_OBJC_OBJECT_POINTER_TYPE,
    VTC_REFERENCE_TYPE,
    VTC_LVALUE_REFERENCE_TYPE,
    VTC_RVALUE_REFERENCE_TYPE,
    VTC_FUNCTION_TYPE_RETURN,
    VTC_ARRAY_TYPE_ELEMENT,
    VTC_TYPE_OF_TYPE,
    VTC_ELABORATED_TYPE,
    VTC_PACK_EXPANSION_TYPE,
    VTC_ATOMIC_TYPE,
    VTC_PIPE_TYPE,
  );

  return "unknown";
}


void ClangASTVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  if (auto dd = dyn_cast<clang::DeclaratorDecl>(decl)) {
    clang::TypeSourceInfo const *tsi = dd->getTypeSourceInfo();
    if (tsi) {
      visitTypeLoc(VTC_DECLARATOR_DECL, tsi->getTypeLoc());
    }
    else {
      visitImplicitQualType(VTC_DECLARATOR_DECL, dd->getType());
    }

    if (clang::Expr const *trailingRequires =
          dd->getTrailingRequiresClause()) {
      visitStmt(VSC_DECLARATOR_DECL_TRAILING_REQUIRES, trailingRequires);
    }

    else if (auto vd = dyn_cast<clang::VarDecl>(decl)) {
      if (clang::Expr const *init = vd->getInit()) {
        visitStmt(VSC_VAR_DECL_INIT, init);
      }
    }

    else if (auto fd = dyn_cast<clang::FunctionDecl>(decl)) {
      visitFunctionDeclParameters(fd);

      if (auto ccd = dyn_cast<clang::CXXConstructorDecl>(decl)) {
        visitCtorInitializers(ccd);
      }

      // For CXXConversionDecl, the TypeLoc is in the return type of the
      // function type, which is visited above as the declarator type.

      if (fd->doesThisDeclarationHaveABody()) {
        clang::Stmt const *body = fd->getBody();
        assert(body);
        visitStmt(VSC_FUNCTION_DECL_BODY, body);
      }
    }

    else if (auto fd = dyn_cast<clang::FieldDecl>(decl)) {
      if (clang::Expr const *width = fd->getBitWidth()) {
        visitStmt(VSC_FIELD_DECL_BIT_WIDTH, width);
      }

      if (clang::Expr const *init = fd->getInClassInitializer()) {
        visitStmt(VSC_FIELD_DECL_INIT, init);
      }
    }

    else if (auto ecd = dyn_cast<clang::EnumConstantDecl>(decl)) {
      if (clang::Expr const *init = ecd->getInitExpr()) {
        visitStmt(VSC_ENUM_CONSTANT_DECL, init);
      }
    }
  }

  else if (auto tnd = dyn_cast<clang::TypedefNameDecl>(decl)) {
    clang::TypeSourceInfo const *tsi = tnd->getTypeSourceInfo();
    assert(tsi);
    visitTypeLoc(VTC_TYPEDEF_NAME_DECL, tsi->getTypeLoc());

    // TypeAliasDecl carries a 'Template' pointer, but I think that
    // points to a parent AST node, and consequently we should not visit
    // it.
  }

  else if (auto ed = dyn_cast<clang::EnumDecl>(decl)) {
    if (clang::TypeSourceInfo *tsi = ed->getIntegerTypeSourceInfo()) {
      visitTypeLoc(VTC_ENUM_DECL_UNDERLYING, tsi->getTypeLoc());
    }

    if (ed->isThisDeclarationADefinition()) {
      visitNonFunctionDeclContext(VDC_ENUM_DECL, DECL_CONTEXT_OF(ed));
    }
  }

  else if (auto fsad = dyn_cast<clang::FileScopeAsmDecl>(decl)) {
    visitStmt(VSC_FILE_SCOPE_ASM_DECL_STRING, fsad->getAsmString());
  }

  // BlockDecl is ObjC I think.

  // TODO: CapturedDecl

  else if (auto rd = dyn_cast<clang::RecordDecl>(decl)) {
    if (rd->isThisDeclarationADefinition()) {
      if (auto crd = dyn_cast<clang::CXXRecordDecl>(decl)) {
        visitCXXRecordBases(crd);
      }
      visitNonFunctionDeclContext(VDC_RECORD_DECL, DECL_CONTEXT_OF(rd));

      // TODO: Visit instantiations of
      // ClassTemplatePartialSpecializationDecl.
    }
  }

  // TODO: CXXDeductionGuideDecl?

  // TODO: BindingDecl, DecompositionDecl

  else if (auto td = dyn_cast<clang::TemplateDecl>(decl)) {
    visitTemplateDeclParameterList(td->getTemplateParameters());
    visitDecl(VDC_TEMPLATE_DECL, td->getTemplatedDecl());

    // TODO: The various TemplateDecl subclasses.

    if (auto ctd = dyn_cast<clang::ClassTemplateDecl>(decl)) {
      visitClassTemplateInstantiations(ctd);
    }

    else if (auto ftd = dyn_cast<clang::FunctionTemplateDecl>(decl)) {
      visitFunctionTemplateInstantiations(ftd);
    }
  }

  else if (auto fd = dyn_cast<clang::FriendDecl>(decl)) {
    clang::TypeSourceInfo const *tsi = fd->getFriendType();
    if (tsi) {
      visitTypeLoc(VTC_FRIEND_DECL, tsi->getTypeLoc());
    }
    else {
      clang::NamedDecl const *inner = fd->getFriendDecl();
      visitDecl(VDC_FRIEND_DECL, inner);
    }
  }

  // The documentation for FriendTemplateDecl says it is not used, and
  // the test at in/src/friend-template-decl.cc appears to confirm that.
  // So the following case is never exercised.
  else if (auto ftd = dyn_cast<clang::FriendTemplateDecl>(decl)) {
    clang::TypeSourceInfo const *tsi = fd->getFriendType();
    if (tsi) {
      visitTypeLoc(VTC_FRIEND_TEMPLATE_DECL, tsi->getTypeLoc());
    }
    else {
      clang::NamedDecl const *inner = fd->getFriendDecl();
      visitDecl(VDC_FRIEND_TEMPLATE_DECL, inner);
    }
  }

  else if (auto ed = dyn_cast<clang::ExportDecl>(decl)) {
    visitNonFunctionDeclContext(VDC_EXPORT_DECL, DECL_CONTEXT_OF(ed));
  }

  else if (auto eccd = dyn_cast<clang::ExternCContextDecl>(decl)) {
    visitNonFunctionDeclContext(VDC_EXTERN_C_DECL, DECL_CONTEXT_OF(eccd));
  }

  else if (auto lsd = dyn_cast<clang::LinkageSpecDecl>(decl)) {
    visitNonFunctionDeclContext(VDC_LINKAGE_SPEC_DECL, DECL_CONTEXT_OF(lsd));
  }

  else if (auto nsd = dyn_cast<clang::NamespaceDecl>(decl)) {
    visitNonFunctionDeclContext(VDC_NAMESPACE_DECL, DECL_CONTEXT_OF(nsd));
  }

  else if (auto rebd = dyn_cast<clang::RequiresExprBodyDecl>(decl)) {
    visitNonFunctionDeclContext(VDC_REQUIRES_EXPR_BODY_DECL, DECL_CONTEXT_OF(rebd));
  }

  else if (auto tud = dyn_cast<clang::TranslationUnitDecl>(decl)) {
    visitNonFunctionDeclContext(VDC_TRANSLATION_UNIT_DECL, DECL_CONTEXT_OF(tud));
  }

  else {
    // Ignore others.
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
  // Initialize the else-if chain.
  if (false) {}

  // Handle a TypeLoc subclass that, for the purpose of this visitor,
  // only contains another TypeLoc inside it that we need to visit.
  #define HANDLE_TYPE_WRAPPER(Subclass, context, accessor)  \
    else if (auto wtl = typeLoc.getAs<clang::Subclass>()) { \
      visitTypeLoc(context, wtl.accessor());                \
    }

  HANDLE_TYPE_WRAPPER(
    QualifiedTypeLoc,
    VTC_QUALIFIED_TYPE,
    getUnqualifiedLoc)

  else if (auto atl = typeLoc.getAs<clang::AttributedTypeLoc>()) {
    // TODO: visitAttr(VAC_ATTRIBUTED_TYPE, atl.getAttr());
    visitTypeLoc(VTC_ATTRIBUTED_TYPE, atl.getModifiedLoc());
  }

  // TODO: ObjCObjectTypeLoc

  // TODO: MacroQualifiedTypeLoc

  HANDLE_TYPE_WRAPPER(
    ParenTypeLoc,
    VTC_PAREN_TYPE,
    getInnerLoc)

  HANDLE_TYPE_WRAPPER(
    AdjustedTypeLoc,
    VTC_ADJUSTED_TYPE,
    getOriginalLoc)

  // Handle any of the concrete subclasses that inherit a specialization
  // of the PointerLikeTypeLoc template class.
  #define HANDLE_POINTER_LIKE_TYPE_LOC(Subclass, context) \
    HANDLE_TYPE_WRAPPER(Subclass, context, getPointeeLoc)

  HANDLE_POINTER_LIKE_TYPE_LOC(
    PointerTypeLoc,
    VTC_POINTER_TYPE)

  HANDLE_POINTER_LIKE_TYPE_LOC(
    BlockPointerTypeLoc,
    VTC_BLOCK_POINTER_TYPE)

  else if (auto mptl = typeLoc.getAs<clang::MemberPointerTypeLoc>()) {
    clang::TypeSourceInfo const *tsi = mptl.getClassTInfo();
    assert(tsi);
    visitTypeLoc(VTC_MEMBER_POINTER_TYPE_CLASS, tsi->getTypeLoc());

    visitTypeLoc(VTC_MEMBER_POINTER_TYPE_POINTEE, mptl.getPointeeLoc());
  }

  HANDLE_POINTER_LIKE_TYPE_LOC(
    ObjCObjectPointerTypeLoc,
    VTC_OBJC_OBJECT_POINTER_TYPE)

  HANDLE_POINTER_LIKE_TYPE_LOC(
    ReferenceTypeLoc,
    VTC_REFERENCE_TYPE)

  HANDLE_POINTER_LIKE_TYPE_LOC(
    LValueReferenceTypeLoc,
    VTC_LVALUE_REFERENCE_TYPE)

  HANDLE_POINTER_LIKE_TYPE_LOC(
    RValueReferenceTypeLoc,
    VTC_RVALUE_REFERENCE_TYPE)

  #undef HANDLE_POINTER_LIKE_TYPE_LOC

  else if (auto ftl = typeLoc.getAs<clang::FunctionTypeLoc>()) {
    visitTypeLoc(VTC_FUNCTION_TYPE_RETURN, ftl.getReturnLoc());

    visitFunctionTypeLocParameters(ftl);

    // I don't know why there isn't an exception spec TypeLoc here to
    // visit.
  }

  else if (auto atl = typeLoc.getAs<clang::ArrayTypeLoc>()) {
    visitTypeLoc(VTC_ARRAY_TYPE_ELEMENT, atl.getElementLoc());

    if (clang::Expr const *size = atl.getSizeExpr()) {
      visitStmt(VSC_ARRAY_TYPE_SIZE, size);
    }
    else {
      // One way this is missing is for an implicitly declared array
      // type such as '__builtin_va_list' that is part of every TU.
    }
  }

  else if (auto tstl = typeLoc.getAs<clang::TemplateSpecializationTypeLoc>()) {
    // I would think there should be a TypeLoc here for the template
    // itself, but I'm not seeing it.  Maybe the template name, alone,
    // is not considered a "type", and hence there is no separate
    // TypeLoc?
    visitTemplateSpecializationTypeLocArguments(tstl);
  }

  /* TODO:

     DependentAddressSpecTypeLoc
     VectorTypeLoc
     DependentVectorTypeLoc
     DependentSizedExtVectorTypeLoc
     MatrixTypeLoc
     ConstantMatrixTypeLoc
     DependentSizedMatrixTypeLoc
     ComplexTypeLoc
  */

  else if (auto toetl = typeLoc.getAs<clang::TypeOfExprTypeLoc>()) {
    visitStmt(VSC_TYPE_OF_TYPE, toetl.getUnderlyingExpr());
  }

  else if (auto totl = typeLoc.getAs<clang::TypeOfTypeLoc>()) {
    clang::TypeSourceInfo const *tsi =
      IF_CLANG_16(totl.getUnmodifiedTInfo(),
                  totl.getUnderlyingTInfo());
    assert(tsi);
    visitTypeLoc(VTC_TYPE_OF_TYPE, tsi->getTypeLoc());
  }

  else if (auto dtl = typeLoc.getAs<clang::DecltypeTypeLoc>()) {
    visitStmt(VSC_DECLTYPE_TYPE, dtl.getUnderlyingExpr());
  }

  // TODO: UnaryTransformTypeLoc

  // I think DeducedTypeLoc does not need anyting.

  // TODO: AutoTypeLoc with template arguments.

  HANDLE_TYPE_WRAPPER(
    ElaboratedTypeLoc,
    VTC_ELABORATED_TYPE,
    getNamedTypeLoc)

  // TODO: DependentTemplateSpecializationTypeLoc template args.

  HANDLE_TYPE_WRAPPER(
    PackExpansionTypeLoc,
    VTC_PACK_EXPANSION_TYPE,
    getPatternLoc)

  HANDLE_TYPE_WRAPPER(
    AtomicTypeLoc,
    VTC_ATOMIC_TYPE,
    getValueLoc)

  HANDLE_TYPE_WRAPPER(
    PipeTypeLoc,
    VTC_PIPE_TYPE,
    getValueLoc)

  #undef HANDLE_TYPE_WRAPPER

  else {
    // For remaining cases, there should be nothing to visit.
  }
}


void ClangASTVisitor::visitImplicitQualType(VisitTypeContext context,
                                            clang::QualType qualType)
{
  // Do nothing.
}


void ClangASTVisitor::visitNonFunctionDeclContext(
  VisitDeclContext context,
  clang::DeclContext const *dc)
{
  for (clang::Decl const *d : dc->decls()) {
    visitDecl(context, d);
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


void ClangASTVisitor::visitFunctionDeclParameters(
  clang::FunctionDecl const *fd)
{
  for (clang::ParmVarDecl const *param : fd->parameters()) {
    visitDecl(VDC_FUNCTION_DECL_PARAMETER, param);
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
  visitTypeLoc(VTC_CXX_RECORD_DECL_BASE, tsi->getTypeLoc());
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
    visitTypeLoc(VTC_CXX_CTOR_INITIALIZER, tsi->getTypeLoc());
  }
  else {
    // The initializer initializes a member (rather than a base, or
    // indicating a delegation), which we do not visit.
  }
}


void ClangASTVisitor::visitTemplateDeclParameterList(
  clang::TemplateParameterList const *tparams)
{
  for (clang::NamedDecl const *param : *tparams) {
    visitDecl(VDC_TEMPLATE_DECL_PARAMETER, param);
  }

  if (clang::Expr const *expr = tparams->getRequiresClause()) {
    visitStmt(VSC_TEMPLATE_DECL_REQUIRES_CLAUSE, expr);
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


void ClangASTVisitor::visitFunctionTypeLocParameters(
  clang::FunctionTypeLoc ftl)
{
  for (clang::ParmVarDecl const *param : ftl.getParams()) {
    visitDecl(VDC_FUNCTION_TYPE_PARAM, param);
  }
}


void ClangASTVisitor::visitTemplateSpecializationTypeLocArguments(
  clang::TemplateSpecializationTypeLoc tstl)
{
  for (unsigned i=0; i < tstl.getNumArgs(); ++i) {
    visitTemplateArgumentLoc(tstl.getArgLoc(i));
  }
}


void ClangASTVisitor::visitTemplateArgumentLoc(
  clang::TemplateArgumentLoc tal)
{
  // TODO
}


// EOF
