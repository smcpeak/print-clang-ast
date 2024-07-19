// clang-ast-visitor.cc
// Code for clang-ast-visitor.h.

#include "clang-ast-visitor.h"                   // this module

// this dir
#include "clang-util.h"                          // assert_dyn_cast
#include "enum-util.h"                           // ENUM_TABLE_LOOKUP_CHECK_SIZE

// smbase
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/gdvsymbol.h"                    // gdv::GDVSymbol
#include "smbase/xassert.h"                      // xassert

// clang
#include "clang/AST/DeclFriend.h"                // clang::{FriendDecl, ...}
#include "clang/AST/ExprCXX.h"                   // clang::{CXXConstructExpr, ...}
#include "clang/AST/ExprConcepts.h"              // clang::concepts::Requirement
#include "clang/AST/StmtCXX.h"                   // clang::{CXXCatchStmt, ...}

using namespace gdv;

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
    VDC_VAR_TEMPLATE_INSTANTIATION,
    VDC_IMPLICIT_FUNCTION_DECL_PARAMETER,
    VDC_TEMPLATE_DECL_PARAMETER,
    VDC_CLASS_SCOPE_FUNCTION_SPECIALIZATION_DECL,

    VDC_FUNCTION_TYPE_PARAMETER,

    VDC_CXX_CATCH_STMT,
    VDC_DECL_STMT,
    VDC_LAMBDA_EXPR_CAPTURE,
    VDC_LAMBDA_EXPR_CLASS,
    VDC_REQUIRES_EXPR_BODY,
    VDC_REQUIRES_EXPR_PARAM,
    VDC_RETURN_STMT_NRVO_CANDIDATE,
  );

  return "unknown";
}

#define DEFINE_TOGDVALUE_USING_TOSTRING(type) \
  gdv::GDValue toGDValue(type value)          \
  {                                           \
    return GDVSymbol(toString(value));        \
  }

DEFINE_TOGDVALUE_USING_TOSTRING(VisitDeclContext)


char const *toString(VisitStmtContext vsc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitStmtContext,
    NUM_VISIT_STMT_CONTEXTS, vsc,

    VSC_NONE,

    VSC_CXX_CTOR_INITIALIZER,
    VSC_DECLARATOR_DECL_TRAILING_REQUIRES,
    VSC_VAR_DECL_INIT,
    VSC_FUNCTION_DECL_BODY,
    VSC_FIELD_DECL_BIT_WIDTH,
    VSC_FIELD_DECL_INIT,
    VSC_ENUM_CONSTANT_DECL,
    VSC_FILE_SCOPE_ASM_DECL_STRING,
    VSC_TEMPLATE_DECL_REQUIRES_CLAUSE,
    VSC_NON_TYPE_TEMPLATE_PARM_DECL_DEFAULT,

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
    VSC_CXX_FOLD_EXPR_CALLEE,
    VSC_CXX_FOLD_EXPR_LHS,
    VSC_CXX_FOLD_EXPR_RHS,
    VSC_CXX_NEW_EXPR_ARRAY_SIZE,
    VSC_CXX_NEW_EXPR_INIT,
    VSC_CXX_NEW_PLACEMENT_ARG,
    VSC_CXX_NOEXCEPT_EXPR,
    VSC_CXX_PSEUDO_DESTRUCTOR_EXPR,
    VSC_CXX_STD_INITIALIZER_LIST_EXPR,
    VSC_CXX_THROW_EXPR,
    VSC_CXX_TYPEID_EXPR,
    VSC_CXX_UUIDOF_EXPR,
    VSC_CXX_UNRESOLVED_CONSTRUCT_EXPR_ARG,
    VSC_CONCEPTS_EXPR_REQUIREMENT,
    VSC_CONCEPTS_NESTED_REQUIREMENT,
    VSC_CONSTANT_EXPR,
    VSC_EXPR_WITH_CLEANUPS,
    VSC_MATERIALIZE_TEMPORARY_EXPR,
    VSC_EXPLICIT_CAST_EXPR,
    VSC_IMPLICIT_CAST_EXPR,
    VSC_CHOOSE_EXPR_COND,
    VSC_CHOOSE_EXPR_LHS,
    VSC_CHOOSE_EXPR_RHS,
    VSC_COMPOUND_LITERAL_EXPR,
    VSC_CONVERT_VECTOR_EXPR,
    VSC_CALL_EXPR_CALLEE,
    VSC_CALL_EXPR_ARG,
    VSC_LAMBDA_EXPR_CAPTURE,
    VSC_MEMBER_EXPR,
    VSC_PAREN_EXPR,
    VSC_PAREN_LIST_EXPR,
    VSC_SUBST_NON_TYPE_TEMPLATE_PARM_EXPR,
    VSC_UNARY_EXPR_OR_TYPE_TRAIT_EXPR,
    VSC_UNARY_OPERATOR,

    VSC_TEMPLATE_ARGUMENT,
  );

  return "unknown";
}

DEFINE_TOGDVALUE_USING_TOSTRING(VisitStmtContext)


char const *toString(VisitTypeContext vtc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitTypeContext,
    NUM_VISIT_TYPE_CONTEXTS, vtc,

    VTC_NONE,

    VTC_DECLARATOR_DECL,
    VTC_TYPEDEF_NAME_DECL,
    VTC_ENUM_DECL_UNDERLYING,
    VTC_CLASS_TEMPLATE_SPECIALIZATION_DECL,
    VTC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL,
    VTC_CXX_RECORD_DECL_BASE,
    VTC_CXX_CTOR_INITIALIZER,
    VTC_FRIEND_DECL,
    VTC_FRIEND_TEMPLATE_DECL,
    VTC_TEMPLATE_TYPE_PARM_DECL_DEFAULT,

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

    VTC_CXX_NEW_EXPR,
    VTC_CXX_PSEUDO_DESTRUCTOR_EXPR_SCOPE,
    VTC_CXX_PSEUDO_DESTRUCTOR_EXPR_DESTROYED,
    VTC_CXX_SCALAR_VALUE_INIT_EXPR,
    VTC_CXX_TYPEID_EXPR,
    VTC_CXX_UNRESOLVED_CONSTRUCT_EXPR,
    VTC_CXX_UUIDOF_EXPR,
    VTC_CXX_TEMPORARY_OBJECT_EXPR,
    VTC_EXPLICIT_CAST_EXPR,
    VTC_COMPOUND_LITERAL_EXPR,
    VTC_CONVERT_VECTOR_EXPR,
    VTC_UNARY_EXPR_OR_TYPE_TRAIT_EXPR,

    VTC_TEMPLATE_TYPE_ARGUMENT,
    VTC_NESTED_NAME_SPECIFIER,
    VTC_DECLARATION_NAME,
    VTC_CONCEPTS_TYPE_REQUIREMENT,
  );

  return "unknown";
}

DEFINE_TOGDVALUE_USING_TOSTRING(VisitTypeContext)


char const *toString(VisitTemplateArgumentContext vtac)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitTemplateArgumentContext,
    NUM_VISIT_TEMPLATE_ARGUMENT_CONTEXTS, vtac,

    VTAC_NONE,

    VTAC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL,
    VTAC_CLASS_SCOPE_FUNCTION_SPECIALIZATION_DECL,
    VTAC_TEMPLATE_TEMPLATE_PARM_DECL_DEFAULT,

    VTAC_TEMPLATE_SPECIALIZATION_TYPE,

    VTAC_CXX_DEPENDENT_SCOPE_MEMBER_EXPR,
    VTAC_DECL_REF_EXPR,
    VTAC_DEPENDENT_SCOPE_DECL_REF_EXPR,
    VTAC_MEMBER_EXPR,
  );

  return "unknown";
}

DEFINE_TOGDVALUE_USING_TOSTRING(VisitTemplateArgumentContext)


char const *toString(VisitNestedNameSpecifierContext vnnsc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitNestedNameSpecifierContext,
    NUM_VISIT_NESTED_NAME_SPECIFIER_CONTEXTS, vnnsc,

    VNNSC_NONE,

    VNNSC_DECLARATOR_DECL,
    VNNSC_ENUM_DECL,
    VNNSC_RECORD_DECL,
    VNNSC_USING_DECL,

    VNNSC_ELABORATED_TYPE,

    VNNSC_CXX_PSEUDO_DESTRUCTOR_EXPR,
    VNNSC_DECL_REF_EXPR,
    VNNSC_DEPENDENT_SCOPE_DECL_REF_EXPR,
    VNNSC_MEMBER_EXPR,
  );

  return "unknown";
}

DEFINE_TOGDVALUE_USING_TOSTRING(VisitNestedNameSpecifierContext)


char const *toString(VisitDeclarationNameContext vdnc)
{
  ENUM_TABLE_LOOKUP_CHECK_SIZE(/*no qual*/, VisitDeclarationNameContext,
    NUM_VISIT_DECLARATION_NAME_CONTEXTS, vdnc,

    VDNC_NONE,

    VDNC_FUNCTION_DECL,

    VDNC_DECL_REF_EXPR,
    VDNC_DEPENDENT_SCOPE_DECL_REF_EXPR,
  );

  return "unknown";
}

DEFINE_TOGDVALUE_USING_TOSTRING(VisitDeclarationNameContext)


// Signal to the check-src Makefile target to stop here: END_OF_ENUMS.
// Without this, it would see call sites below that start with a context
// enumerator due to line wrapping.


ClangASTVisitor::ClangASTVisitor()
{}


void ClangASTVisitor::scanTU(clang::ASTContext &astContext)
{
  visitDecl(VDC_NONE, astContext.getTranslationUnitDecl());
}


bool ClangASTVisitor::shouldVisitInstantiationsAfterDefinitions() const
{
  return false;
}


void ClangASTVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  // This function currently uses an else-if chain rather than a
  // 'switch' statement for ease of understanding and maintenance.  I
  // envision, once it stabilizes, converting it to a 'switch'.

  if (auto dd = dyn_cast<clang::DeclaratorDecl>(decl)) {
    visitDeclaratorDeclOuterTemplateParameters(dd);

    // To me, it makes more sense to visit the type before visiting the
    // NNS since (in traditional syntax at least) the type syntactically
    // precedes the NNS, but RecursiveASTVisitor visits the NNS first,
    // and I want to match its behavior to make comparing them easier
    // during testing, so I will visit the NNS first too.
    visitNestedNameSpecifierLocOpt(VNNSC_DECLARATOR_DECL,
                                   dd->getQualifierLoc());

    if (auto fd = dyn_cast<clang::FunctionDecl>(decl)) {
      // To match the RAV order, visit the DeclarationNameInfo of a
      // FunctionDecl up here.
      visitDeclarationNameInfo(VDNC_FUNCTION_DECL,
                               fd->getNameInfo());
    }

    // Note that visiting the type usually includes visiting the
    // parameters if this is a declaration of a function.
    visitDeclaratorDeclType(dd);

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
      if (!fd->getTypeSourceInfo()) {
        // There is no TypeLoc, so the parameters do not get visited
        // above.  Visit them now.
        //
        // Originlly, this code checked 'isImplicit()', but the
        // '__invoke' method on a lambda class that does *not* have any
        // captures is considered 'isImplicit()' but it still has
        // TypeSourceInfo.
        //
        // There might be a more principled approach.  For the moment,
        // I'm just trying to match what RAV does.
        visitImplicitFunctionDeclParameters(fd);
      }

      if (auto ccd = dyn_cast<clang::CXXConstructorDecl>(decl)) {
        if (ccd->doesThisDeclarationHaveABody()) {
          visitCXXCtorInitializers(ccd);
        }
      }

      // For CXXConversionDecl, the TypeLoc is in the return type of the
      // function type, which is visited above as the declarator type.
      // It is also in the DeclarationNameInfo, visited separately
      // above.

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

    else if (auto nttpd = dyn_cast<clang::NonTypeTemplateParmDecl>(decl)) {
      if (nttpd->hasDefaultArgument() &&
          !nttpd->defaultArgumentWasInherited()) {
        visitStmt(VSC_NON_TYPE_TEMPLATE_PARM_DECL_DEFAULT,
          nttpd->getDefaultArgument());
      }
    }
  }

  else if (auto ecd = dyn_cast<clang::EnumConstantDecl>(decl)) {
    if (clang::Expr const *init = ecd->getInitExpr()) {
      visitStmt(VSC_ENUM_CONSTANT_DECL, init);
    }
  }

  else if (auto tnd = dyn_cast<clang::TypedefNameDecl>(decl)) {
    visitTypeSourceInfo(VTC_TYPEDEF_NAME_DECL, tnd->getTypeSourceInfo());

    // TypeAliasDecl carries a 'Template' pointer, but I think that
    // points to a parent AST node, and consequently we should not visit
    // it.
  }

  // Both EnumDecl and RecordDecl inherit TagDecl, but they visit their
  // NestedNameSpecifiers in different places (for RAV compatibility),
  // so I can't easily combine their cases.  And, splitting them
  // provides an opportunity to refine the NNS context slightly.

  else if (auto ed = dyn_cast<clang::EnumDecl>(decl)) {
    visitTagDeclOuterTemplateParameters(ed);
    visitNestedNameSpecifierLocOpt(VNNSC_ENUM_DECL, ed->getQualifierLoc());

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
    bool const isDefn = rd->isThisDeclarationADefinition();

    if (auto crd = dyn_cast<clang::CXXRecordDecl>(decl)) {
      if (auto ctsd = dyn_cast<
            clang::ClassTemplateSpecializationDecl>(crd)) {
        if (auto ctpsd = dyn_cast<
              clang::ClassTemplatePartialSpecializationDecl>(decl)) {
          visitTemplateDeclParameterList(
            ctpsd->getTemplateParameters());

          if (false) {
            // CTPSD has 'ArgsAsWritten' that are redundant with the
            // 'TypeAsWritten'.  Do not visit the former.
            visitASTTemplateArgumentListInfo(
              VTAC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL,
              ctpsd->getTemplateArgsAsWritten());
          }

          // Visit 'TypeAsWritten', but with a special context so I
          // can behave like RAV when needed.
          visitTypeSourceInfoOpt(
            VTC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL,
            ctsd->getTypeAsWritten());
        }
        else /*full specialization*/ {
          visitTypeSourceInfoOpt(
            VTC_CLASS_TEMPLATE_SPECIALIZATION_DECL,
            ctsd->getTypeAsWritten());
        }
      }
    }

    // It would make much more sense to visit the outer parameters
    // before the inner parameters, but RAV does it here.
    visitTagDeclOuterTemplateParameters(rd);

    // RAV prints the NNS after the template parameters and arguments.
    // Syntactically, it appears between them, but RAV compatibility
    // is important for my testing strategy.  (It would also be a bit
    // awkward to insert the NNS in there, which is presumably why RAV
    // does what it does in this regard.)
    visitNestedNameSpecifierLocOpt(VNNSC_RECORD_DECL, rd->getQualifierLoc());

    // CXXRecord has two sections to handle it because that is needed
    // to match the RAV visitation order.
    if (auto crd = dyn_cast<clang::CXXRecordDecl>(decl)) {
      if (isDefn) {
        visitCXXRecordBases(crd);
      }
    }

    if (isDefn) {
      visitNonFunctionDeclContext(VDC_RECORD_DECL, DECL_CONTEXT_OF(rd));
    }

    if (auto ctpsd = dyn_cast<
          clang::ClassTemplatePartialSpecializationDecl>(decl)) {
      if (isDefn && shouldVisitInstantiationsAfterDefinitions()) {
        visitClassTemplatePartialSpecializationInstantiations(ctpsd);
      }

      // If there is no definition, then clang will not associate any
      // specializations with the partial spec, so we do not need to
      // fall back on visiting after the canonical the way we do for the
      // template primary.
    }
  } // RecordDecl

  else if (auto fsad = dyn_cast<clang::FileScopeAsmDecl>(decl)) {
    visitStmt(VSC_FILE_SCOPE_ASM_DECL_STRING, fsad->getAsmString());
  }

  // BlockDecl is ObjC I think.

  // TODO: CapturedDecl

  // TODO: CXXDeductionGuideDecl?

  // TODO: BindingDecl, DecompositionDecl

  else if (auto td = dyn_cast<clang::TemplateDecl>(decl)) {
    visitTemplateDeclParameterList(td->getTemplateParameters());

    // The templated decl is missing in the case of a
    // TemplateTemplateParmDecl.
    visitDeclOpt(VDC_TEMPLATE_DECL, td->getTemplatedDecl());

    visitTemplateInstantiationsIfAppropriate(td);

    if (auto ttpd = dyn_cast<clang::TemplateTemplateParmDecl>(decl)) {
      if (ttpd->hasDefaultArgument() &&
          !ttpd->defaultArgumentWasInherited()) {
        visitTemplateArgumentLoc(VTAC_TEMPLATE_TEMPLATE_PARM_DECL_DEFAULT,
          ttpd->getDefaultArgument());
      }
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
    clang::TypeSourceInfo const *tsi = ftd->getFriendType();
    if (tsi) {
      visitTypeLoc(VTC_FRIEND_TEMPLATE_DECL, tsi->getTypeLoc());
    }
    else {
      clang::NamedDecl const *inner = ftd->getFriendDecl();
      visitDecl(VDC_FRIEND_TEMPLATE_DECL, inner);
    }
  }

  else if (auto csfsd = dyn_cast<
             clang::ClassScopeFunctionSpecializationDecl>(decl)) {
    // Visit the specialization class.
    visitDecl(
      VDC_CLASS_SCOPE_FUNCTION_SPECIALIZATION_DECL,
      csfsd->getSpecialization());

    // Visit the template arguments.
    visitASTTemplateArgumentListInfoOpt(
      VTAC_CLASS_SCOPE_FUNCTION_SPECIALIZATION_DECL,
      csfsd->getTemplateArgsAsWritten());
  }

  else if (auto ttpd = dyn_cast<clang::TemplateTypeParmDecl>(decl)) {
    // TODO: Type constraint?

    // Visit the default argument if it is syntactically present.
    if (ttpd->hasDefaultArgument() &&
        !ttpd->defaultArgumentWasInherited()) {
      visitTypeSourceInfo(VTC_TEMPLATE_TYPE_PARM_DECL_DEFAULT,
        ttpd->getDefaultArgumentInfo());
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

  else if (auto ud = dyn_cast<clang::UsingDecl>(decl)) {
    visitNestedNameSpecifierLocOpt(VNNSC_USING_DECL, ud->getQualifierLoc());
  }

  else {
    // Ignore others.
  }
}


void ClangASTVisitor::visitStmt(VisitStmtContext context,
                                clang::Stmt const *origStmt)
{
  // The order of cases in this 'switch' statement is meant to
  // correspond to the numeric order of the 'Stmt::StmtClass'
  // enumerators except where adjustment is needed because of the
  // class hierarchy.
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

    // After a series of ADDITIONAL_STMT_CLASS, this begins handling
    // them all as the indicated superclass.  It does not have its own
    // case label because the abstract superclasses do not have their
    // own codes, as they are never instantiated.
    #define BEGIN_STMT_ABSTRACT_SUPERCLASS(Superclass)  \
      /*relevant cases come before this*/ {             \
        clang::Superclass const *stmt =                 \
          assert_dyn_cast(clang::Superclass, origStmt);

    // Although the AsmStmt subclasses contain string literals, their
    // semantics are completely different from other kinds of string
    // literals, so I think it's best to not recursively traverse into
    // it.
    BEGIN_NOOP_STMT_CLASS(GCCAsmStmt)
    HANDLE_NOOP_STMT_CLASS(MSAsmStmt)

    HANDLE_NOOP_STMT_CLASS(BreakStmt)

    HANDLE_STMT_CLASS(CXXCatchStmt)
      // Optional because there is no declaration for `catch (...)`.
      visitDeclOpt(VDC_CXX_CATCH_STMT, stmt->getExceptionDecl());

      visitStmt   (VSC_CXX_CATCH_STMT, stmt->getHandlerBlock());

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

    // TODO: All the OMP* statements.
    // TODO: All the ObjC* statements.

    HANDLE_STMT_CLASS(ReturnStmt)
      visitStmtOpt(VSC_RETURN_STMT_VALUE, stmt->getRetValue());
      visitDeclOpt(VDC_RETURN_STMT_NRVO_CANDIDATE, stmt->getNRVOCandidate());

    // TODO: SEHExceptStmt
    // TODO: SEHFinallyStmt
    // TODO: SEHLeaveStmt
    // TODO: SEHTryStmt

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

    // CompoundAssignOperator is a subclass of BinaryOperator that adds
    // some results of semantic analysis but no new syntax.
    END_STMT_CLASS
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

    HANDLE_STMT_CLASS(CXXFoldExpr)
      // TODO: I ran into a case
      // (`header-analysis/in/src/int32-via-memory.h`) where the callee
      // was `nullptr`, so I now tolerate all of them being null.  I
      // should figure out what actually is possible.
      visitStmtOpt(VSC_CXX_FOLD_EXPR_CALLEE, stmt->getCallee());
      visitStmtOpt(VSC_CXX_FOLD_EXPR_LHS, stmt->getLHS());
      visitStmtOpt(VSC_CXX_FOLD_EXPR_RHS, stmt->getRHS());

    HANDLE_NOOP_STMT_CLASS(CXXInheritedCtorInitExpr)

    HANDLE_STMT_CLASS(CXXNewExpr)
      visitTypeSourceInfo(VTC_CXX_NEW_EXPR,
        stmt->getAllocatedTypeSourceInfo());
      if (stmt->isArray()) {
        visitStmt(VSC_CXX_NEW_EXPR_ARRAY_SIZE,
          *(stmt->getArraySize()));
      }
      if (stmt->hasInitializer()) {
        visitStmt(VSC_CXX_NEW_EXPR_INIT,
          stmt->getInitializer());
      }
      visitCXXNewExprPlacementArgs(stmt);

    HANDLE_STMT_CLASS(CXXNoexceptExpr)
      visitStmt(VSC_CXX_NOEXCEPT_EXPR,
        stmt->getOperand());

    HANDLE_NOOP_STMT_CLASS(CXXNullPtrLiteralExpr)

    // TODO: CXXParenListInitExpr (C++20 feature)

    HANDLE_STMT_CLASS(CXXPseudoDestructorExpr)
      visitStmt(
        VSC_CXX_PSEUDO_DESTRUCTOR_EXPR,
        stmt->getBase());
      visitNestedNameSpecifierLocOpt(
        VNNSC_CXX_PSEUDO_DESTRUCTOR_EXPR,
        stmt->getQualifierLoc());
      visitTypeSourceInfoOpt(
        VTC_CXX_PSEUDO_DESTRUCTOR_EXPR_SCOPE,
        stmt->getScopeTypeInfo());
      visitTypeSourceInfoOpt(
        VTC_CXX_PSEUDO_DESTRUCTOR_EXPR_DESTROYED,
        stmt->getDestroyedTypeInfo());

    // TODO: CXXRewrittenBinaryOperator (C++20 feature)

    HANDLE_STMT_CLASS(CXXScalarValueInitExpr)
      visitTypeSourceInfo(VTC_CXX_SCALAR_VALUE_INIT_EXPR,
        stmt->getTypeSourceInfo());

    HANDLE_STMT_CLASS(CXXStdInitializerListExpr)
      visitStmt(VSC_CXX_STD_INITIALIZER_LIST_EXPR,
        stmt->getSubExpr());

    HANDLE_NOOP_STMT_CLASS(CXXThisExpr)

    HANDLE_STMT_CLASS(CXXThrowExpr)
      visitStmtOpt(VSC_CXX_THROW_EXPR,
        stmt->getSubExpr());

    HANDLE_STMT_CLASS(CXXTypeidExpr)
      if (stmt->isTypeOperand()) {
        visitTypeSourceInfo(VTC_CXX_TYPEID_EXPR,
          stmt->getTypeOperandSourceInfo());
      }
      else {
        visitStmt(VSC_CXX_TYPEID_EXPR,
          stmt->getExprOperand());
      }

    HANDLE_STMT_CLASS(CXXUnresolvedConstructExpr)
      visitTypeSourceInfo(VTC_CXX_UNRESOLVED_CONSTRUCT_EXPR,
        stmt->getTypeSourceInfo());
      visitCXXUnresolvedConstructExprArgs(stmt);

    HANDLE_STMT_CLASS(CXXUuidofExpr)
      if (stmt->isTypeOperand()) {
        visitTypeSourceInfo(VTC_CXX_UUIDOF_EXPR,
          stmt->getTypeOperandSourceInfo());
      }
      else {
        visitStmt(VSC_CXX_UUIDOF_EXPR,
          stmt->getExprOperand());
      }

    END_STMT_CLASS
    ADDITIONAL_STMT_CLASS(CUDAKernelCallExpr)     // TODO: This is incomplete, there is the "config" too.
    ADDITIONAL_STMT_CLASS(CXXMemberCallExpr)
    ADDITIONAL_STMT_CLASS(CXXOperatorCallExpr)
    ADDITIONAL_STMT_CLASS(UserDefinedLiteral)
    BEGIN_STMT_CLASS(CallExpr)
      visitStmt(VSC_CALL_EXPR_CALLEE, stmt->getCallee());
      visitCallExprArgs(stmt);

    END_STMT_CLASS
    ADDITIONAL_STMT_CLASS(BuiltinBitCastExpr)
    ADDITIONAL_STMT_CLASS(CStyleCastExpr)
    ADDITIONAL_STMT_CLASS(CXXFunctionalCastExpr)
    ADDITIONAL_STMT_CLASS(CXXAddrspaceCastExpr)
    ADDITIONAL_STMT_CLASS(CXXConstCastExpr)
    ADDITIONAL_STMT_CLASS(CXXDynamicCastExpr)
    ADDITIONAL_STMT_CLASS(CXXReinterpretCastExpr)
    ADDITIONAL_STMT_CLASS(CXXStaticCastExpr)
    BEGIN_STMT_ABSTRACT_SUPERCLASS(ExplicitCastExpr)
      visitTypeSourceInfo(
        VTC_EXPLICIT_CAST_EXPR,
        stmt->getTypeInfoAsWritten());
      visitStmt(
        VSC_EXPLICIT_CAST_EXPR,
        stmt->getSubExpr());

    // TODO: ObjCBridgedCastExprClass

    HANDLE_STMT_CLASS(ImplicitCastExpr)
      // There is no TypeLoc for an implicit cast.  I could call
      // 'visitImplicitQualType' here, but I would just be visiting the
      // expression's type, which does not provide much value over
      // letting the client do the same.

      visitStmt(VSC_IMPLICIT_CAST_EXPR, stmt->getSubExpr());

    HANDLE_NOOP_STMT_CLASS(CharacterLiteral)

    HANDLE_STMT_CLASS(ChooseExpr)
      visitStmt(VSC_CHOOSE_EXPR_COND, stmt->getCond());
      visitStmt(VSC_CHOOSE_EXPR_LHS, stmt->getLHS());
      visitStmt(VSC_CHOOSE_EXPR_RHS, stmt->getRHS());

    HANDLE_STMT_CLASS(CompoundLiteralExpr)
      visitTypeSourceInfoOrImplicitQualType(
        VTC_COMPOUND_LITERAL_EXPR,
        stmt->getTypeSourceInfo(),
        stmt->getType());

      // There is a "FIXME" comment in Expr.h indicating that the
      // initializer might be nullptr, but "should" not be.  I'll be
      // defensive and check.
      visitStmtOpt(VSC_COMPOUND_LITERAL_EXPR,
        stmt->getInitializer());

    // TODO: ConceptSpecializationExpr (C++20 feature?)

    HANDLE_STMT_CLASS(ConvertVectorExpr)
      visitStmt(VSC_CONVERT_VECTOR_EXPR,
        stmt->getSrcExpr());

      // RAV does not do this...
      visitTypeSourceInfo(VTC_CONVERT_VECTOR_EXPR,
        stmt->getTypeSourceInfo());

    // TODO: CoawaitExpr
    // TODO: CoyieldExpr

    HANDLE_STMT_CLASS(DeclRefExpr)
      visitNestedNameSpecifierLocOpt(
        VNNSC_DECL_REF_EXPR,
        stmt->getQualifierLoc());
      visitDeclarationNameInfo(
        VDNC_DECL_REF_EXPR,
        stmt->getNameInfo());
      visitTemplateArgumentLocArray(
        VTAC_DECL_REF_EXPR,
        stmt->getTemplateArgs(),
        stmt->getNumTemplateArgs());

    // TODO: DependentCoawaitExpr

    HANDLE_STMT_CLASS(DependentScopeDeclRefExpr)
      visitNestedNameSpecifierLoc(
        VNNSC_DEPENDENT_SCOPE_DECL_REF_EXPR,
        stmt->getQualifierLoc());
      visitDeclarationNameInfo(
        VDNC_DEPENDENT_SCOPE_DECL_REF_EXPR,
        stmt->getNameInfo());
      visitTemplateArgumentLocArray(
        VTAC_DEPENDENT_SCOPE_DECL_REF_EXPR,
        stmt->getTemplateArgs(),
        stmt->getNumTemplateArgs());

    // TODO: DesignatedInitExpr
    // TODO: DesignatedInitUpdateExpr

    // TODO: ExpressionTraitExpr

    // TODO: ExtVectorElementExpr

    // TODO: FixedPointLiteral
    // TODO: FloatingLiteral

    HANDLE_STMT_CLASS(ConstantExpr)
      visitStmt(VSC_CONSTANT_EXPR, stmt->getSubExpr());

    // This shares a superclass with 'ConstantExpr', namely 'FullExpr',
    // but I want to have two different contexts, so I do not combine
    // their cases.
    HANDLE_STMT_CLASS(ExprWithCleanups)
      visitStmt(VSC_EXPR_WITH_CLEANUPS, stmt->getSubExpr());

    // TODO: FunctionParmPackExpr
    // TODO: GNUNullExpr
    // TODO: GenericSelectionExpr
    // TODO: ImaginaryLiteral
    // TODO: ImplicitValueInitExpr
    // TODO: InitListExpr
    // TODO: IntegerLiteral

    HANDLE_STMT_CLASS(LambdaExpr)
      visitLambdaExprCaptures(stmt);

      // Each syntactic occurrence of a lambda will generate a unique
      // lambda class, so it should be regarded as a child node.  This
      // class contains all of the other syntactic elements, including
      // the lambda body (as a method definition).
      visitDecl(VDC_LAMBDA_EXPR_CLASS, stmt->getLambdaClass());

    // TODO: MSPropertyRefExpr
    // TODO: MSPropertySubscriptExpr

    HANDLE_STMT_CLASS(MaterializeTemporaryExpr)
      visitStmt(VSC_MATERIALIZE_TEMPORARY_EXPR, stmt->getSubExpr());

    // TODO: MatrixSubscriptExpr

    HANDLE_STMT_CLASS(MemberExpr)
      visitNestedNameSpecifierLocOpt(
        VNNSC_MEMBER_EXPR,
        stmt->getQualifierLoc());
      visitTemplateArgumentLocArray(
        VTAC_MEMBER_EXPR,
        stmt->getTemplateArgs(),
        stmt->getNumTemplateArgs());

      // Visit this last for RAV compatibility.
      visitStmt(
        VSC_MEMBER_EXPR,
        stmt->getBase());

    // TODO: NoInitExpr

    // TODO: OMP*Expr
    // TODO: ObjC*Expr

    // TODO: OffsetOfExpr
    // TODO: OpaqueValueExpr
    // TODO: UnresolvedLookupExpr
    // TODO: UnresolvedMemberExpr
    // TODO: PackExpansionExpr

    HANDLE_STMT_CLASS(ParenExpr)
      visitStmt(
        VSC_PAREN_EXPR,
        stmt->getSubExpr());

    HANDLE_STMT_CLASS(ParenListExpr)
      visitParenListExprExprs(stmt);

    // TODO: PredefinedExpr
    // TODO: PseudoObjectExpr
    // TODO: RecoveryExpr

    HANDLE_STMT_CLASS(RequiresExpr)
      // Basically just copying RAV here.
      visitDecl(VDC_REQUIRES_EXPR_BODY, stmt->getBody());
      visitRequiresExprParameters(stmt);
      visitRequiresExprRequirements(stmt);

    // TODO: SYCLUniqueStableNameExpr
    // TODO: ShuffleVectorExpr
    // TODO: SizeOfPackExpr
    // TODO: SourceLocExpr
    // TODO: StmtExpr
    // TODO: StringLiteral

    HANDLE_STMT_CLASS(SubstNonTypeTemplateParmExpr)
      visitStmt(
        VSC_SUBST_NON_TYPE_TEMPLATE_PARM_EXPR,
        stmt->getReplacement());

    // TODO: SubstNonTypeTemplateParmPackExpr
    // TODO: TypeTraitExpr
    // TODO: TypoExpr

    HANDLE_STMT_CLASS(UnaryExprOrTypeTraitExpr)
      if (stmt->isArgumentType()) {
        visitTypeSourceInfo(
          VTC_UNARY_EXPR_OR_TYPE_TRAIT_EXPR,
          stmt->getArgumentTypeInfo());
      }
      else {
        visitStmt(
          VSC_UNARY_EXPR_OR_TYPE_TRAIT_EXPR,
          stmt->getArgumentExpr());
      }

    HANDLE_STMT_CLASS(UnaryOperator)
      visitStmt(
        VSC_UNARY_OPERATOR,
        stmt->getSubExpr());

    // TODO: VAArgExpr

    // TODO: LabelStmt
    // TODO: WhileStmt

    END_STMT_CLASS

    #undef BEGIN_STMT_CLASS
    #undef END_STMT_CLASS
    #undef HANDLE_STMT_CLASS
    #undef BEGIN_NOOP_STMT_CLASS
    #undef HANDLE_NOOP_STMT_CLASS
    #undef ADDITIONAL_STMT_CLASS
    #undef BEGIN_STMT_ABSTRACT_SUPERCLASS

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
  // Similar to 'visitDecl', I envision at some point converting this to
  // use a 'switch' instead of an else-if chain.

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

  else if (auto etl = typeLoc.getAs<clang::ElaboratedTypeLoc>()) {
    visitNestedNameSpecifierLocOpt(
      VNNSC_ELABORATED_TYPE,
      etl.getQualifierLoc());
    visitTypeLoc(
      VTC_ELABORATED_TYPE,
      etl.getNamedTypeLoc());
  }

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
      visitTypeSourceInfo(VTC_TEMPLATE_TYPE_ARGUMENT, tal.getTypeSourceInfo());
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


void ClangASTVisitor::visitDeclarationNameInfo(
  VisitDeclarationNameContext context,
  clang::DeclarationNameInfo dni)
{
  if (clang::TypeSourceInfo const *tsi = dni.getNamedTypeInfo()) {
    visitTypeSourceInfo(VTC_DECLARATION_NAME, tsi);
  }
}


void ClangASTVisitor::visitConceptsRequirement(
  clang::concepts::Requirement const *req)
{
  switch (req->getKind()) {
    case clang::concepts::Requirement::RK_Type: {
      auto tr = assert_dyn_cast(clang::concepts::TypeRequirement, req);
      if (!tr->isSubstitutionFailure()) {
        visitTypeSourceInfo(VTC_CONCEPTS_TYPE_REQUIREMENT,
          tr->getType());
      }
      break;
    }

    case clang::concepts::Requirement::RK_Simple:
    case clang::concepts::Requirement::RK_Compound: {
      auto er = assert_dyn_cast(clang::concepts::ExprRequirement, req);
      if (!er->isExprSubstitutionFailure()) {
        visitStmt(VSC_CONCEPTS_EXPR_REQUIREMENT,
          er->getExpr());
      }
      break;
    }

    case clang::concepts::Requirement::RK_Nested: {
      auto nr = assert_dyn_cast(clang::concepts::NestedRequirement, req);
      if (!nr->IF_CLANG_16(hasInvalidConstraint(),
                           isSubstitutionFailure())) {
        visitStmt(VSC_CONCEPTS_NESTED_REQUIREMENT,
          nr->getConstraintExpr());
      }
      break;
    }

    // The above cases are exhaustive.
  }
}


void ClangASTVisitor::visitFunctionTemplateInstantiations(
  clang::FunctionTemplateDecl const *ftd)
{
  for (clang::FunctionDecl const *spec : ftd->specializations()) {
    if (spec->isTemplateInstantiation()) {
      // Function template specializations get one redeclaration for
      // each redeclaration of the primary, and any of them could be
      // where the definition is.
      for (clang::FunctionDecl const *redecl : spec->redecls()) {
        visitDecl(VDC_FUNCTION_TEMPLATE_INSTANTIATION, redecl);
      }
    }
  }
}


// Like `ClassTemplateDecl::getInstantiatedFrom`, except returning a
// single `NamedDecl` instead of a pointer union.  Returns null for
// something that is not an instantiation.
static clang::NamedDecl const * NULLABLE getInstFromNamedDecl(
  clang::ClassTemplateSpecializationDecl const *spec)
{
  // This is an `llvm::PointerUnion`.
  auto instFromPU = spec->getInstantiatedFrom();

  if (auto instFromCTD = instFromPU.dyn_cast<clang::ClassTemplateDecl *>()) {
    return instFromCTD;
  }
  else {
    // This could return null for a non-instantiation.
    return instFromPU.dyn_cast<clang::ClassTemplatePartialSpecializationDecl *>();
  }
}


bool ClangASTVisitor::isInstantiationOfThisClassTemplateOrPartial(
  clang::ClassTemplateSpecializationDecl const *spec,
  clang::NamedDecl const *ctopd) const
{
  clang::NamedDecl const * NULLABLE instFromDecl =
    getInstFromNamedDecl(spec);
  xassert(instFromDecl);
  return instFromDecl->getCanonicalDecl() ==
         ctopd->getCanonicalDecl();
}


void ClangASTVisitor::visitClassTemplateInstantiations(
  clang::ClassTemplateDecl const *ctd,
  bool onlyInstantiationsOfThisTemplate)
{
  for (clang::ClassTemplateSpecializationDecl const *spec :
         ctd->specializations()) {
    // TODO: RecursiveASTVisitor traverses the redeclarations of `spec`
    // in the equivalent place.  It also does that for other kinds of
    // templates.

    clang::TemplateSpecializationKind tsk =
      spec->getSpecializationKind();

    if (tsk == clang::TSK_Undeclared) {
      // This could be a specialization that is nominated by a deduction
      // guide but not otherwise instantiatied or explicitly
      // specialized.  It could also be a specialization mentioned but
      // not instantiable because the relevant template has not been
      // defined.  Since there is no instantiated-from, we will visit it
      // with the primary.
      visitDecl(VDC_CLASS_TEMPLATE_INSTANTIATION, spec);
    }

    if (tsk == clang::TSK_ImplicitInstantiation) {
      if (onlyInstantiationsOfThisTemplate &&
          !isInstantiationOfThisClassTemplateOrPartial(spec, ctd)) {
        // `spec` is an instantiation of something else (which must be a
        // partial specialization), so skip it here.
        continue;
      }
      visitDecl(VDC_CLASS_TEMPLATE_INSTANTIATION, spec);
    }
  }
}


void ClangASTVisitor::visitClassTemplatePartialSpecializationInstantiations(
  clang::ClassTemplatePartialSpecializationDecl const *ctpsd)
{
  clang::ClassTemplateDecl const *primary =
    ctpsd->getSpecializedTemplate();

  for (clang::ClassTemplateSpecializationDecl const *spec :
         primary->specializations()) {
    clang::TemplateSpecializationKind tsk =
      spec->getSpecializationKind();
    if (tsk == clang::TSK_ImplicitInstantiation) {
      if (isInstantiationOfThisClassTemplateOrPartial(spec, ctpsd)) {
        visitDecl(VDC_CLASS_TEMPLATE_INSTANTIATION, spec);
      }
    }
  }
}


void ClangASTVisitor::visitVarTemplateInstantiations(
  clang::VarTemplateDecl const *vtd)
{
  for (clang::VarTemplateSpecializationDecl const *spec :
         vtd->specializations()) {
    if (clang::isTemplateInstantiation(spec->getSpecializationKind())) {
      visitDecl(VDC_VAR_TEMPLATE_INSTANTIATION, spec);
    }
  }
}


void ClangASTVisitor::visitCXXCtorInitializer(
  clang::CXXCtorInitializer const *init)
{
  // Note: If it is skipping implicit code, and 'init->isWritten()' is
  // false, RecursiveASTVisitor will skip the initializer expression,
  // but not the type.  I do not see any reason for that inconsistency.
  // (I'm noting it here for lack of any better place.)

  if (clang::TypeSourceInfo const *tsi = init->getTypeSourceInfo()) {
    visitTypeLoc(VTC_CXX_CTOR_INITIALIZER, tsi->getTypeLoc());
  }
  else {
    // The initializer initializes a member (rather than a base, or
    // indicating a delegation), which we do not visit.
  }

  visitStmt(VSC_CXX_CTOR_INITIALIZER, init->getInit());
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


void ClangASTVisitor::visitTypeSourceInfoOpt(
  VisitTypeContext context,
  clang::TypeSourceInfo const * NULLABLE tsi)
{
  if (tsi) {
    visitTypeSourceInfo(context, tsi);
  }
}


void ClangASTVisitor::visitNonFunctionDeclContext(
  VisitDeclContext context,
  clang::DeclContext const *dc)
{
  for (clang::Decl const *d : dc->decls()) {
    if (auto crd = dyn_cast<clang::CXXRecordDecl>(d)) {
      if (crd->isLambda()) {
        // The `crd` object will be visited when its originating
        // `LambdaExpr` is visited.  Therefore, skip it here.  (This
        // matches the behavior of `RecursiveASTVisitor`.)
        continue;
      }
    }

    visitDecl(context, d);
  }
}


void ClangASTVisitor::visitImplicitFunctionDeclParameters(
  clang::FunctionDecl const *fd)
{
  for (clang::ParmVarDecl const *param : fd->parameters()) {
    visitDecl(VDC_IMPLICIT_FUNCTION_DECL_PARAMETER, param);
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


void ClangASTVisitor::visitCXXCtorInitializers(
  clang::CXXConstructorDecl const *ccd)
{
  for (clang::CXXCtorInitializer const *init : ccd->inits()) {
    visitCXXCtorInitializer(init);
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


void ClangASTVisitor::visitASTTemplateArgumentListInfo(
  VisitTemplateArgumentContext context,
  clang::ASTTemplateArgumentListInfo const *argListInfo)
{
  visitTemplateArgumentLocArray(
    context,
    argListInfo->getTemplateArgs(),
    argListInfo->getNumTemplateArgs());
}


void ClangASTVisitor::visitASTTemplateArgumentListInfoOpt(
  VisitTemplateArgumentContext context,
  clang::ASTTemplateArgumentListInfo const * NULLABLE argListInfo)
{
  if (argListInfo) {
    visitASTTemplateArgumentListInfo(context, argListInfo);
  }
}


void ClangASTVisitor::visitFunctionTypeLocParameters(
  clang::FunctionTypeLoc ftl)
{
  for (clang::ParmVarDecl const *param : ftl.getParams()) {
    visitDecl(VDC_FUNCTION_TYPE_PARAMETER, param);
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


void ClangASTVisitor::visitCallExprArgs(
  clang::CallExpr const *callExpr)
{
  for (clang::Expr const *arg : callExpr->arguments()) {
    visitStmt(VSC_CALL_EXPR_ARG, arg);
  }
}


void ClangASTVisitor::visitTemplateInstantiationsIfAppropriate(
  clang::TemplateDecl const *templateDecl)
{
  if (auto ctd = dyn_cast<clang::ClassTemplateDecl>(templateDecl)) {
    visitClassTemplateInstantiationsIfAppropriate(ctd);
  }

  else if (auto ftd = dyn_cast<clang::FunctionTemplateDecl>(templateDecl)) {
    visitFunctionTemplateInstantiationsIfAppropriate(ftd);
  }

  else if (auto vtd = dyn_cast<clang::VarTemplateDecl>(templateDecl)) {
    visitVarTemplateInstantiationsIfAppropriate(vtd);
  }
}


// `TEMPLATE_DECL` is one of:
//
//   * ClassTemplateDecl
//   * FunctionTemplateDecl
//   * VarTemplateDecl
//
// It is tempting to visit the instantiations when `templateDecl` is the
// definition rather than canonical.  The problem with that is if there
// is no definition, then there are still instantiations of the
// declaration, and can be instantiations of partial specializations.
// And even when there is a definition, there can be instantiations of
// the declaration only.  Only an instantiation of the definition is
// necessarily tied to the template definition.
//
template <typename TEMPLATE_DECL>
bool ClangASTVisitor::isAppropriateForInstantiations(
  TEMPLATE_DECL const *templateDecl) const
{
  if (shouldVisitInstantiationsAfterDefinitions()) {
    // Does this template have a definition?  Unfortunately, only for
    // `VarTemplateDecl` can we ask that directly, so we instead go to
    // the body declaration, which is one of `CXXRecordDecl`,
    // `FunctionDecl`, or `VarDecl`.
    auto bodyDecl = templateDecl->getTemplatedDecl();
    if (bodyDecl->getDefinition()) {
      // There is a definition, so visit instantiations when
      // `templateDecl` is that definition.
      return bodyDecl->isThisDeclarationADefinition();
    }
    else {
      // There is no definition, so visit after the canonical.
      return templateDecl->isCanonicalDecl();
    }
  }

  else {
    // Like RAV, always visit instantiations after the canonical, even
    // though that will often mean visiting them before visiting the
    // template from which they were instantiated.
    return templateDecl->isCanonicalDecl();
  }
}


void ClangASTVisitor::visitFunctionTemplateInstantiationsIfAppropriate(
  clang::FunctionTemplateDecl const *ftd)
{
  if (isAppropriateForInstantiations(ftd)) {
    visitFunctionTemplateInstantiations(ftd);
  }
}


void ClangASTVisitor::visitClassTemplateInstantiationsIfAppropriate(
  clang::ClassTemplateDecl const *ctd)
{
  if (isAppropriateForInstantiations(ctd)) {
    visitClassTemplateInstantiations(ctd,
      shouldVisitInstantiationsAfterDefinitions());
  }
}


void ClangASTVisitor::visitVarTemplateInstantiationsIfAppropriate(
  clang::VarTemplateDecl const *vtd)
{
  if (isAppropriateForInstantiations(vtd)) {
    visitVarTemplateInstantiations(vtd);
  }
}


void ClangASTVisitor::visitLambdaExprCapture(
  clang::LambdaExpr const *lambdaExpr,
  clang::LambdaCapture const *capture,
  clang::Expr const *init)
{
  // I don't really understand this code, especially what the difference
  // is between the two cases.  I'm basically copying
  // RecursiveASTVisitor<Derived>::TraverseLambdaCapture() here.
  if (lambdaExpr->isInitCapture(capture)) {
    visitDecl(VDC_LAMBDA_EXPR_CAPTURE, capture->getCapturedVar());
  }
  else {
    visitStmt(VSC_LAMBDA_EXPR_CAPTURE, init);
  }
}


void ClangASTVisitor::visitDeclaratorDeclOuterTemplateParameters(
  clang::DeclaratorDecl const *dd)
{
  for (unsigned i=0; i < dd->getNumTemplateParameterLists(); ++i) {
    visitTemplateDeclParameterList(dd->getTemplateParameterList(i));
  }
}


void ClangASTVisitor::visitTagDeclOuterTemplateParameters(
  clang::TagDecl const *td)
{
  for (unsigned i=0; i < td->getNumTemplateParameterLists(); ++i) {
    visitTemplateDeclParameterList(td->getTemplateParameterList(i));
  }
}


void ClangASTVisitor::visitCXXNewExprPlacementArgs(
  clang::CXXNewExpr const *newExpr)
{
  for (unsigned i=0; i < newExpr->getNumPlacementArgs(); ++i) {
    visitStmt(VSC_CXX_NEW_PLACEMENT_ARG,
      newExpr->getPlacementArg(i));
  }
}


void ClangASTVisitor::visitCXXUnresolvedConstructExprArgs(
  clang::CXXUnresolvedConstructExpr const *constructExpr)
{
  for (unsigned i=0; i < constructExpr->getNumArgs(); ++i) {
    visitStmt(VSC_CXX_UNRESOLVED_CONSTRUCT_EXPR_ARG,
      constructExpr->getArg(i));
  }
}


void ClangASTVisitor::visitTypeSourceInfoOrImplicitQualType(
  VisitTypeContext context,
  clang::TypeSourceInfo const * NULLABLE tsi,
  clang::QualType qualType)
{
  if (tsi) {
    visitTypeSourceInfo(context, tsi);
  }
  else {
    visitImplicitQualType(context, qualType);
  }
}


void ClangASTVisitor::visitLambdaExprCaptures(
  clang::LambdaExpr const *lambdaExpr)
{
  // This does not iterate over 'captures()' because we want to provide
  // the initializer expressions at the same time.
  for (unsigned i=0; i < lambdaExpr->capture_size(); ++i) {
    // It's odd that pointer arithmetic is the only way to index a
    // particular element.
    clang::LambdaCapture const *capture = lambdaExpr->capture_begin() + i;
    clang::Expr const *init = lambdaExpr->capture_init_begin()[i];
    visitLambdaExprCapture(lambdaExpr, capture, init);
  }
}


void ClangASTVisitor::visitParenListExprExprs(
  clang::ParenListExpr const *parenListExpr)
{
  for (unsigned i=0; i < parenListExpr->getNumExprs(); ++i) {
    visitStmt(VSC_PAREN_LIST_EXPR, parenListExpr->getExpr(i));
  }
}


void ClangASTVisitor::visitRequiresExprParameters(
  clang::RequiresExpr const *requiresExpr)
{
  for (clang::ParmVarDecl const *param :
         requiresExpr->getLocalParameters()) {
    visitDecl(VDC_REQUIRES_EXPR_PARAM, param);
  }
}


void ClangASTVisitor::visitRequiresExprRequirements(
  clang::RequiresExpr const *requiresExpr)
{
  for (clang::concepts::Requirement const *req :
         requiresExpr->getRequirements()) {
    visitConceptsRequirement(req);
  }
}


void ClangASTVisitor::visitDeclaratorDeclType(
  clang::DeclaratorDecl const *dd)
{
  visitTypeSourceInfoOrImplicitQualType(
    VTC_DECLARATOR_DECL,
    dd->getTypeSourceInfo(),
    dd->getType());
}


// EOF
