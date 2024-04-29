// clang-ast-visitor.cc
// Code for clang-ast-visitor.h.

#include "clang-ast-visitor.h"                   // this module

// this dir
#include "clang-util.h"                          // assert_dyn_cast
#include "enum-util.h"                           // ENUM_TABLE_LOOKUP_CHECK_SIZE

// clang
#include "clang/AST/DeclFriend.h"                // clang::{FriendDecl, ...}
#include "clang/AST/ExprCXX.h"                   // clang::{CXXConstructExpr, ...}
#include "clang/AST/StmtCXX.h"                   // clang::{CXXCatchStmt, ...}

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

    VDC_CXX_CATCH_STMT,
    VDC_DECL_STMT,
    VDC_RETURN_STMT_NRVO_CANDIDATE,
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

    VSC_CXX_CATCH_STMT,
    VSC_CXX_FOR_RANGE_STMT_INIT,
    VSC_CXX_FOR_RANGE_STMT_RANGE,
    VSC_CXX_FOR_RANGE_STMT_BEGIN,
    VSC_CXX_FOR_RANGE_STMT_END,
    VSC_CXX_FOR_RANGE_STMT_COND,
    VSC_CXX_FOR_RANGE_STMT_INC,
    VSC_CXX_FOR_RANGE_STMT_LOOPVAR,
    VSC_CXX_FOR_RANGE_STMT_BODY,
    VSC_CXX_TRY_STMT_TRY_BLOCK,
    VSC_CXX_TRY_STMT_HANDLER,
    VSC_COMPOUND_STMT,
    VSC_DO_STMT_BODY,
    VSC_DO_STMT_COND,
    VSC_FOR_STMT_INIT,
    VSC_FOR_STMT_CONDVAR,
    VSC_FOR_STMT_COND,
    VSC_FOR_STMT_INC,
    VSC_FOR_STMT_BODY,
    VSC_IF_STMT_INIT,
    VSC_IF_STMT_CONDVAR,
    VSC_IF_STMT_COND,
    VSC_IF_STMT_THEN,
    VSC_IF_STMT_ELSE,
    VSC_INDIRECT_GOTO_STMT,
    VSC_RETURN_STMT_VALUE,
    VSC_CASE_STMT_LHS,
    VSC_CASE_STMT_RHS,
    VSC_CASE_STMT_SUB,
    VSC_DEFAULT_STMT,
    VSC_SWITCH_STMT_INIT,
    VSC_SWITCH_STMT_CONDVAR,
    VSC_SWITCH_STMT_COND,
    VSC_SWITCH_STMT_BODY,
    VSC_ATTRIBUTED_STMT,
    VSC_BINARY_CONDITIONAL_OPERATOR_COMMON,
    VSC_BINARY_CONDITIONAL_OPERATOR_COND,
    VSC_BINARY_CONDITIONAL_OPERATOR_TRUE,
    VSC_BINARY_CONDITIONAL_OPERATOR_FALSE,
    VSC_CONDITIONAL_OPERATOR_COND,
    VSC_CONDITIONAL_OPERATOR_TRUE,
    VSC_CONDITIONAL_OPERATOR_FALSE,
    VSC_ARRAY_INIT_LOOP_EXPR_COMMON,
    VSC_ARRAY_INIT_LOOP_EXPR_SUB,
    VSC_ARRAY_SUBSCRIPT_EXPR_LHS,
    VSC_ARRAY_SUBSCRIPT_EXPR_RHS,
    VSC_AS_TYPE_EXPR,
    VSC_BINARY_OPERATOR_LHS,
    VSC_BINARY_OPERATOR_RHS,
    VSC_CXX_BIND_TEMPORARY_EXPR,
    VSC_CXX_CONSTRUCT_EXPR,
    VSC_CXX_DELETE_EXPR,
    VSC_CXX_DEPENDENT_SCOPE_MEMBER_EXPR_BASE,
    VSC_CAST_EXPR,

    VSC_TEMPLATE_ARGUMENT,
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

    VTC_CXX_TEMPORARY_OBJECT_EXPR,

    VTC_TEMPLATE_ARGUMENT,
    VTC_NESTED_NAME_SPECIFIER,
  );

  return "unknown";
}


char const *toString(VisitTemplateArgumentContext vtac)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitTemplateArgumentContext,
    NUM_VISIT_TEMPLATE_ARGUMENT_CONTEXTS, vtac,

    VTAC_NONE,

    VTAC_TEMPLATE_SPECIALIZATION_TYPE,

    VTAC_CXX_DEPENDENT_SCOPE_MEMBER_EXPR,
    VTAC_DECL_REF_EXPR,
  );

  return "unknown";
}


char const *toString(VisitNestedNameSpecifierContext vnnsc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitNestedNameSpecifierContext,
    NUM_VISIT_NESTED_NAME_SPECIFIER_CONTEXTS, vnnsc,

    VNNSC_NONE,

    VNNSC_DECLARATOR_DECL,
    VNNSC_TAG_DECL,
  );

  return "unknown";
}


void ClangASTVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  if (auto dd = dyn_cast<clang::DeclaratorDecl>(decl)) {
    // To me, it makes more sense to visit the type before visiting the
    // NNS since (in traditional syntax at least) the type syntactically
    // precedes the NNS, but RecursiveASTVisitor visits the NNS first,
    // and I want to match its behavior to make comparing them easier
    // during testing, so I will visit the NNS first too.
    visitNestedNameSpecifierLocOpt(VNNSC_DECLARATOR_DECL,
                                   dd->getQualifierLoc());

    if (clang::TypeSourceInfo const *tsi = dd->getTypeSourceInfo()) {
      visitTypeLoc(VTC_DECLARATOR_DECL, tsi->getTypeLoc());
    }
    else {
      visitImplicitQualType(VTC_DECLARATOR_DECL, dd->getType());
    }

    if (clang::Expr const *trailingRequires =
          dd->getTrailingRequiresClause()) {
      visitStmt(VSC_DECLARATOR_DECL_TRAILING_REQUIRES, trailingRequires);
    }

    if (auto vd = dyn_cast<clang::VarDecl>(decl)) {
      if (clang::Expr const *init = vd->getInit()) {
        visitStmt(VSC_VAR_DECL_INIT, init);
      }
    }

    else if (auto fd = dyn_cast<clang::FunctionDecl>(decl)) {
      visitFunctionDeclParameters(fd);

      if (auto ccd = dyn_cast<clang::CXXConstructorDecl>(decl)) {
        if (ccd->doesThisDeclarationHaveABody()) {
          visitCtorInitializers(ccd);
        }
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
    visitTypeSourceInfo(VTC_TYPEDEF_NAME_DECL, tnd->getTypeSourceInfo());

    // TypeAliasDecl carries a 'Template' pointer, but I think that
    // points to a parent AST node, and consequently we should not visit
    // it.
  }

  else if (auto td = dyn_cast<clang::TagDecl>(decl)) {
    visitNestedNameSpecifierLocOpt(VNNSC_TAG_DECL, td->getQualifierLoc());

    if (auto ed = dyn_cast<clang::EnumDecl>(decl)) {
      if (clang::TypeSourceInfo const *tsi = ed->getIntegerTypeSourceInfo()) {
        visitTypeLoc(VTC_ENUM_DECL_UNDERLYING, tsi->getTypeLoc());
      }
      else {
        // The declaration does not have an underlying type declared.
      }

      if (ed->isThisDeclarationADefinition()) {
        visitNonFunctionDeclContext(VDC_ENUM_DECL, DECL_CONTEXT_OF(ed));
      }
    }

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
  }

  else if (auto fsad = dyn_cast<clang::FileScopeAsmDecl>(decl)) {
    visitStmt(VSC_FILE_SCOPE_ASM_DECL_STRING, fsad->getAsmString());
  }

  // BlockDecl is ObjC I think.

  // TODO: CapturedDecl

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
                                clang::Stmt const *origStmt)
{
  switch (origStmt->getStmtClass()) {
    // Start handling a particular class.  This deliberately opens a
    // compound statement that it does not close.
    #define BEGIN_STMT_CLASS(Subclass)                \
      case clang::Stmt::Subclass##Class: {            \
        clang::Subclass const *stmt =                 \
          assert_dyn_cast(clang::Subclass, origStmt);

    // End the handling of a class, closing its compound statement.
    #define END_STMT_CLASS \
        break;             \
      }

    // End the previous case and start a new one.
    #define HANDLE_STMT_CLASS(Subclass) \
      END_STMT_CLASS                    \
      BEGIN_STMT_CLASS(Subclass)

    // Begin a case that does not require any code.
    #define BEGIN_NOOP_STMT_CLASS(Subclass) \
      case clang::Stmt::Subclass##Class: {

    // Handle a noop case in the middle of the chain.
    #define HANDLE_NOOP_STMT_CLASS(Subclass) \
      END_STMT_CLASS                         \
      BEGIN_NOOP_STMT_CLASS(Subclass)

    // A class whose handling is the same as what follows it.  You have
    // to explicitly end the previous class first, and follow this with
    // BEGIN rather than HANDLE.
    #define ADDITIONAL_STMT_CLASS(Subclass) \
      case clang::Stmt::Subclass##Class:

    // Although the AsmStmt subclasses contain string literals, their
    // semantics are completely different from other kinds of string
    // literals, so I think it's best to not recursively traverse into
    // it.
    BEGIN_NOOP_STMT_CLASS(GCCAsmStmt)
    HANDLE_NOOP_STMT_CLASS(MSAsmStmt)

    HANDLE_NOOP_STMT_CLASS(BreakStmt)

    HANDLE_STMT_CLASS(CXXCatchStmt)
      visitDecl(VDC_CXX_CATCH_STMT, stmt->getExceptionDecl());
      visitStmt(VSC_CXX_CATCH_STMT, stmt->getHandlerBlock());

    HANDLE_STMT_CLASS(CXXForRangeStmt)
      // I don't know what all of these sub-expressions mean, so I don't
      // know if it makes sense to visit them all.  This is just a first
      // attempt.
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_INIT, stmt->getInit());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_RANGE, stmt->getRangeStmt());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_BEGIN, stmt->getBeginStmt());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_END, stmt->getEndStmt());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_COND, stmt->getCond());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_INC, stmt->getInc());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_LOOPVAR, stmt->getLoopVarStmt());
      visitStmtOpt(VSC_CXX_FOR_RANGE_STMT_BODY, stmt->getBody());

    HANDLE_STMT_CLASS(CXXTryStmt)
      visitStmt(VSC_CXX_TRY_STMT_TRY_BLOCK, stmt->getTryBlock());
      visitCXXTryStmtHandlers(stmt);

    // TODO: HANDLE_STMT_CLASS(CapturedStmt)

    HANDLE_STMT_CLASS(CompoundStmt)
      visitCompoundStmtBody(stmt);

    HANDLE_NOOP_STMT_CLASS(ContinueStmt)

    // TODO: HANDLE_STMT_CLASS(CoreturnStmt)
    // TODO: HANDLE_STMT_CLASS(CoroutineBodyStmt)

    HANDLE_STMT_CLASS(DeclStmt)
      visitDeclStmtDecls(stmt);

    HANDLE_STMT_CLASS(DoStmt)
      visitStmt(VSC_DO_STMT_BODY, stmt->getBody());
      visitStmt(VSC_DO_STMT_COND, stmt->getCond());

    HANDLE_STMT_CLASS(ForStmt)
      visitStmtOpt(VSC_FOR_STMT_INIT, stmt->getInit());
      visitStmtOpt(VSC_FOR_STMT_CONDVAR, stmt->getConditionVariableDeclStmt());
      visitStmtOpt(VSC_FOR_STMT_COND, stmt->getCond());
      visitStmtOpt(VSC_FOR_STMT_INC, stmt->getInc());
      visitStmt   (VSC_FOR_STMT_BODY, stmt->getBody());

    HANDLE_NOOP_STMT_CLASS(GotoStmt)

    HANDLE_STMT_CLASS(IfStmt)
      visitStmtOpt(VSC_IF_STMT_INIT, stmt->getInit());
      visitStmtOpt(VSC_IF_STMT_CONDVAR, stmt->getConditionVariableDeclStmt());
      visitStmt   (VSC_IF_STMT_COND, stmt->getCond());
      visitStmt   (VSC_IF_STMT_THEN, stmt->getThen());
      visitStmtOpt(VSC_IF_STMT_ELSE, stmt->getElse());

    HANDLE_STMT_CLASS(IndirectGotoStmt)
      visitStmt(VSC_INDIRECT_GOTO_STMT, stmt->getTarget());

    // TODO: HANDLE_STMT_CLASS(MSDependentExistsStmt)

    HANDLE_NOOP_STMT_CLASS(NullStmt)

    // TODO: All the OMP* stuff.

    // TODO: All the ObjC* stuff.

    HANDLE_STMT_CLASS(ReturnStmt)
      visitStmt(VSC_RETURN_STMT_VALUE, stmt->getRetValue());
      visitDeclOpt(VDC_RETURN_STMT_NRVO_CANDIDATE, stmt->getNRVOCandidate());

    // TODO: SEHExceptStmtClass,
    // TODO: SEHFinallyStmtClass,
    // TODO: SEHLeaveStmtClass,
    // TODO: SEHTryStmtClass,

    HANDLE_STMT_CLASS(CaseStmt)
      visitStmt   (VSC_CASE_STMT_LHS, stmt->getLHS());
      visitStmtOpt(VSC_CASE_STMT_RHS, stmt->getRHS());
      visitStmt   (VSC_CASE_STMT_SUB, stmt->getSubStmt());

    HANDLE_STMT_CLASS(DefaultStmt)
      visitStmt(VSC_DEFAULT_STMT, stmt->getSubStmt());

    HANDLE_STMT_CLASS(SwitchStmt)
      visitStmtOpt(VSC_SWITCH_STMT_INIT, stmt->getInit());
      visitStmtOpt(VSC_SWITCH_STMT_CONDVAR, stmt->getConditionVariableDeclStmt());
      visitStmt   (VSC_SWITCH_STMT_COND, stmt->getCond());
      visitStmt   (VSC_SWITCH_STMT_BODY, stmt->getBody());

    HANDLE_STMT_CLASS(AttributedStmt)
      // TODO: Visit the attributes.
      visitStmt(VSC_ATTRIBUTED_STMT, stmt->getSubStmt());

    HANDLE_STMT_CLASS(BinaryConditionalOperator)
      visitStmt(VSC_BINARY_CONDITIONAL_OPERATOR_COMMON,
                  stmt->getCommon());
      visitStmt(VSC_BINARY_CONDITIONAL_OPERATOR_COND,
                  stmt->getCond());
      visitStmt(VSC_BINARY_CONDITIONAL_OPERATOR_TRUE,
                  stmt->getTrueExpr());
      visitStmt(VSC_BINARY_CONDITIONAL_OPERATOR_FALSE,
                  stmt->getFalseExpr());

    HANDLE_STMT_CLASS(ConditionalOperator)
      visitStmt(VSC_CONDITIONAL_OPERATOR_COND, stmt->getCond());
      visitStmt(VSC_CONDITIONAL_OPERATOR_TRUE, stmt->getTrueExpr());
      visitStmt(VSC_CONDITIONAL_OPERATOR_FALSE, stmt->getFalseExpr());

    // This class has a pointer to a LabelDecl, but the LabelDecl is not
    // a child, as it is defined elsewhere.  This is merely a reference
    // to that label.  Consequently, I do not traverse into it.
    HANDLE_NOOP_STMT_CLASS(AddrLabelExpr)

    // This expression represents the "current index" within an
    // ArrayInitLoopExpr.  It does not carry any information other than
    // its identity as the expression playing that role.
    HANDLE_NOOP_STMT_CLASS(ArrayInitIndexExpr)

    HANDLE_STMT_CLASS(ArrayInitLoopExpr)
      visitStmt(VSC_ARRAY_INIT_LOOP_EXPR_COMMON, stmt->getCommonExpr());
      visitStmt(VSC_ARRAY_INIT_LOOP_EXPR_SUB, stmt->getSubExpr());

    HANDLE_STMT_CLASS(ArraySubscriptExpr)
      // I choose to visit in syntactic order because a visitor is not
      // likely to be helped by the base/index ordering.
      visitStmt(VSC_ARRAY_SUBSCRIPT_EXPR_LHS, stmt->getLHS());
      visitStmt(VSC_ARRAY_SUBSCRIPT_EXPR_RHS, stmt->getRHS());

    // TODO: ArrayTypeTraitExpr

    HANDLE_STMT_CLASS(AsTypeExpr)
      visitStmt(VSC_AS_TYPE_EXPR, stmt->getSrcExpr());

    // TODO: AtomicExpr

    END_STMT_CLASS

    // CompoundAssignOperator is a subclass of BinaryOperator that adds
    // some results of semantic analysis but no new syntax.
    ADDITIONAL_STMT_CLASS(CompoundAssignOperator)

    BEGIN_STMT_CLASS(BinaryOperator)
      visitStmt(VSC_BINARY_OPERATOR_LHS, stmt->getLHS());
      visitStmt(VSC_BINARY_OPERATOR_RHS, stmt->getRHS());

    // TODO: BlockExpr

    HANDLE_STMT_CLASS(CXXBindTemporaryExpr)
      // The contained 'CXXTemporary' doesn't have recursive structure,
      // so I don't think it needs to be visited.
      visitStmt(VSC_CXX_BIND_TEMPORARY_EXPR, stmt->getSubExpr());

    HANDLE_NOOP_STMT_CLASS(CXXBoolLiteralExpr)

    HANDLE_STMT_CLASS(CXXConstructExpr)
      visitCXXConstructExprArgs(stmt);

    // This is a subclass of 'CXXConstructExpr', so could be folded into
    // the case above, but doing so would be messier than repeating one
    // line of code.
    HANDLE_STMT_CLASS(CXXTemporaryObjectExpr)
      visitTypeSourceInfo(VTC_CXX_TEMPORARY_OBJECT_EXPR, stmt->getTypeSourceInfo());
      visitCXXConstructExprArgs(stmt);

    // These have pointers to elsewhere in the AST, but the pointees are
    // not descendants of these nodes.
    HANDLE_NOOP_STMT_CLASS(CXXDefaultArgExpr)
    HANDLE_NOOP_STMT_CLASS(CXXDefaultInitExpr)

    HANDLE_STMT_CLASS(CXXDeleteExpr)
      visitStmt(VSC_CXX_DELETE_EXPR, stmt->getArgument());

    HANDLE_STMT_CLASS(CXXDependentScopeMemberExpr)
      if (!stmt->isImplicitAccess()) {
        visitStmt(VSC_CXX_DEPENDENT_SCOPE_MEMBER_EXPR_BASE,
                    stmt->getBase());
      }
      // TODO: visitNestedNameSpecifierLoc(stmt->getQualifierLoc());
      visitTemplateArgumentLocArray(
        VTAC_CXX_DEPENDENT_SCOPE_MEMBER_EXPR,
        stmt->getTemplateArgs(),
        stmt->getNumTemplateArgs());

    /*
      TODO: Working on these:

      CXXFoldExpr
      CXXInheritedCtorInitExpr
      CXXNewExpr
      CXXNoexceptExpr
      CXXNullPtrLiteralExpr
      CXXParenListInitExpr
      CXXPseudoDestructorExpr
      CXXRewrittenBinaryOperator
      CXXScalarValueInitExpr
      CXXStdInitializerListExpr
      CXXThisExpr
      CXXThrowExpr
      CXXTypeidExpr
      CXXUnresolvedConstructExpr
      CXXUuidofExpr
      CallExpr
      CUDAKernelCallExpr
      CXXMemberCallExpr
      CXXOperatorCallExpr
      UserDefinedLiteral
      firstCallExprConstant=CallExprClass, lastCallExprConstant=UserDefinedLiteralClass,
      BuiltinBitCastExpr
      CStyleCastExpr
      CXXFunctionalCastExpr
      CXXAddrspaceCastExpr
      CXXConstCastExpr
      CXXDynamicCastExpr
      CXXReinterpretCastExpr
      CXXStaticCastExpr

      plus more I have not copied over
    */

    // Jumping ahead...
    HANDLE_STMT_CLASS(DeclRefExpr)
      // TODO: visitNestedNameSpecifierLoc(getQualifierLoc()).
      visitTemplateArgumentLocArray(
        VTAC_DECL_REF_EXPR,
        stmt->getTemplateArgs(),
        stmt->getNumTemplateArgs());

    HANDLE_STMT_CLASS(ImplicitCastExpr)
      visitStmt(VSC_CAST_EXPR, stmt->getSubExpr());


    END_STMT_CLASS

    #undef BEGIN_STMT_CLASS
    #undef END_STMT_CLASS
    #undef HANDLE_STMT_CLASS
    #undef BEGIN_NOOP_STMT_CLASS
    #undef HANDLE_NOOP_STMT_CLASS
    #undef ADDITIONAL_STMT_CLASS

    // Eventually I hope the cases will be exhaustive, but that's a long
    // way off with all the OMP and ObjC stuff, so we just ignore
    // unrecognized stuff.
    default:
      break;
  }
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
    visitTypeSourceInfo(VTC_MEMBER_POINTER_TYPE_CLASS, mptl.getClassTInfo());
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
    visitTypeSourceInfo(VTC_TYPE_OF_TYPE,
      IF_CLANG_16(totl.getUnmodifiedTInfo(),
                  totl.getUnderlyingTInfo()));
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


void ClangASTVisitor::visitTemplateArgumentLoc(
  VisitTemplateArgumentContext context,
  clang::TemplateArgumentLoc tal)
{
  switch (tal.getArgument().getKind()) {
    case clang::TemplateArgument::Null:
      // Does not contain any information.  And getting here would mean
      // violating this function's precondition.
      break;

    case clang::TemplateArgument::Type:
      visitTypeSourceInfo(VTC_TEMPLATE_ARGUMENT, tal.getTypeSourceInfo());
      break;

    case clang::TemplateArgument::Declaration:
      visitStmt(VSC_TEMPLATE_ARGUMENT,
                tal.getSourceDeclExpression());
      break;

    case clang::TemplateArgument::NullPtr:
      visitStmt(VSC_TEMPLATE_ARGUMENT,
                tal.getSourceNullPtrExpression());
      break;

    case clang::TemplateArgument::Integral:
      visitStmt(VSC_TEMPLATE_ARGUMENT,
                tal.getSourceIntegralExpression());
      break;

    case clang::TemplateArgument::Template:
      // TODO
      break;

    case clang::TemplateArgument::TemplateExpansion:
      // TODO
      break;

    case clang::TemplateArgument::Expression:
      visitStmt(VSC_TEMPLATE_ARGUMENT,
                tal.getSourceExpression());
      break;

    case clang::TemplateArgument::Pack:
      // TODO
      break;

    // The above cases should be exhaustive, so no 'default' here.
  }
}


void ClangASTVisitor::visitNestedNameSpecifierLoc(
  VisitNestedNameSpecifierContext context,
  clang::NestedNameSpecifierLoc nnsl)
{
  // Start by visiting the prefix if there is one.
  visitNestedNameSpecifierLocOpt(context, nnsl.getPrefix());

  // Now examine the qualifier here.
  visitNestedNameSpecifierLocFinalComponent(nnsl);
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


void ClangASTVisitor::visitImplicitQualType(VisitTypeContext context,
                                            clang::QualType qualType)
{
  // Do nothing.
}


void ClangASTVisitor::visitTypeSourceInfo(
  VisitTypeContext context,
  clang::TypeSourceInfo const *tsi)
{
  assert(tsi);
  visitTypeLoc(context, tsi->getTypeLoc());
}


void ClangASTVisitor::visitNonFunctionDeclContext(
  VisitDeclContext context,
  clang::DeclContext const *dc)
{
  for (clang::Decl const *d : dc->decls()) {
    visitDecl(context, d);
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
  visitTypeSourceInfo(VTC_CXX_RECORD_DECL_BASE, base.getTypeSourceInfo());
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


void ClangASTVisitor::visitTemplateArgumentLocArray(
  VisitTemplateArgumentContext context,
  clang::TemplateArgumentLoc const *args,
  unsigned numArgs)
{
  for (unsigned i=0; i < numArgs; ++i) {
    visitTemplateArgumentLoc(context, args[i]);
  }
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
    visitTemplateArgumentLoc(VTAC_TEMPLATE_SPECIALIZATION_TYPE,
                             tstl.getArgLoc(i));
  }
}


void ClangASTVisitor::visitCXXTryStmtHandlers(
  clang::CXXTryStmt const *stmt)
{
  for (unsigned i=0; i < stmt->getNumHandlers(); ++i) {
    visitStmt(VSC_CXX_TRY_STMT_HANDLER, stmt->getHandler(i));
  }
}


void ClangASTVisitor::visitCompoundStmtBody(
  clang::CompoundStmt const *compound)
{
  for (clang::Stmt const *stmt : compound->body()) {
    visitStmt(VSC_COMPOUND_STMT, stmt);
  }
}


void ClangASTVisitor::visitDeclStmtDecls(
  clang::DeclStmt const *declStmt)
{
  for (clang::Decl const *decl : declStmt->decls()) {
    visitDecl(VDC_DECL_STMT, decl);
  }
}


void ClangASTVisitor::visitCXXConstructExprArgs(
  clang::CXXConstructExpr const *cexpr)
{
  for (clang::Expr const *arg : cexpr->arguments()) {
    visitStmt(VSC_CXX_CONSTRUCT_EXPR, arg);
  }
}


void ClangASTVisitor::visitNestedNameSpecifierLocOpt(
  VisitNestedNameSpecifierContext context,
  clang::NestedNameSpecifierLoc nnsl)
{
  if (nnsl.hasQualifier()) {
    visitNestedNameSpecifierLoc(context, nnsl);
  }
}


void ClangASTVisitor::visitNestedNameSpecifierLocFinalComponent(
  clang::NestedNameSpecifierLoc nnsl)
{
  clang::NestedNameSpecifier const *nns = nnsl.getNestedNameSpecifier();
  assert(nns);
  switch (nns->getKind()) {
    case clang::NestedNameSpecifier::Identifier:
      // It would not be unreasonable to add a method to allow direct
      // visitation of IdentifierInfo, but I have no need currently.
      break;

    case clang::NestedNameSpecifier::Namespace:
    case clang::NestedNameSpecifier::NamespaceAlias:
    case clang::NestedNameSpecifier::Global:
    case clang::NestedNameSpecifier::Super:
      // All of these cases are leaves from the perspective of
      // traversal.
      break;

    case clang::NestedNameSpecifier::TypeSpec:
    case clang::NestedNameSpecifier::TypeSpecWithTemplate:
      // Visit the contained type description.
      visitTypeLoc(VTC_NESTED_NAME_SPECIFIER, nnsl.getTypeLoc());
      break;

    // The cases should be exhaustive, so no 'default'.
  }
}


// EOF
