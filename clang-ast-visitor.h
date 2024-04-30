// clang-ast-visitor.h
// ClangASTVisitor class.

#ifndef CLANG_AST_VISITOR_H
#define CLANG_AST_VISITOR_H

// this dir
#include "clang-decl-context-fwd.h"              // clang::DeclContext [n]
#include "clang-decl-cxx-fwd.h"                  // clang::CXXBaseSpecifier [n]
#include "clang-decl-template-fwd.h"             // clang::TemplateParameterList [n]
#include "clang-declaration-name-fwd.h"          // clang::DeclarationNameInfo [n]
#include "clang-nested-name-specifier-fwd.h"     // clang::NestedNameSpecifierLoc [n]
#include "clang-template-base-fwd.h"             // clang::TemplateArgument [n]
#include "clang-type-fwd.h"                      // clang::QualType [n]
#include "clang-type-loc-fwd.h"                  // clang::TypeLoc [n]
#include "util-macros.h"                         // NULLABLE

// clang
#include "clang/AST/ASTFwd.h"                    // clang::{Stmt, Decl, ...} [n]


/*
  Possible roles a Decl can have in the AST.

  These contexts provide a crude partitioning of Decl roles.  A context
  is passed to the 'visitDecl' method to make it easier for clients to
  do basic filtering based on role.  However, it is impossible to
  convey every possible interesting aspect of the syntactic context in
  a fixed-size data structure, so while it is hoped that this context
  will prove useful in some cases, clients that need more will have to
  resort to maintaining their own additional state during traversal.
  (Fortunately, the visitor design makes doing so relatively easy.)

  The naming convention is to begin with the name of the Clang AST class
  that contains the Decl, or a similar name when the context is not
  directly tied to a class, and then an optional further noun describing
  the Decl role within that class if there is more than one or if the
  role is otherwise not obvious.

  The context enumerator order is only partially structured.  Decl,
  Type, Stmt, and <other> contexts are grouped.  Within groups, it's
  partly alphabetical, partly the order in which I happened to implement
  them, partly logical association, and what's leftover is basically
  random.  But, of course, they must match the order in
  clang-ast-visitor.cc, and there is a Makefile target "check-src" that
  ensures that (and static_asserts that confirm the counts agree).
*/
enum VisitDeclContext {
  // Used when initiating a traversal from outside the visitor.
  VDC_NONE,

  // ---- Context is a Decl ----
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
  VDC_IMPLICIT_FUNCTION_DECL_PARAMETER,
  VDC_TEMPLATE_DECL_PARAMETER,   // also used for ClassTemplatePartialSpecializationDecl parameters
  VDC_CLASS_SCOPE_FUNCTION_SPECIALIZATION_DECL,

  // ---- Context is a TypeLoc ----
  VDC_FUNCTION_TYPE_PARAMETER,

  // ---- Context is a Stmt ----
  VDC_CXX_CATCH_STMT,
  VDC_DECL_STMT,
  VDC_RETURN_STMT_NRVO_CANDIDATE,

  NUM_VISIT_DECL_CONTEXTS
};

// Return a string like "VDC_NONE", or "unknown" if 'vdc' is invalid.
char const *toString(VisitDeclContext vdc);


// Possible roles for a Stmt.
enum VisitStmtContext {
  VSC_NONE,

  // ---- Context is a Decl ----
  VSC_DECLARATOR_DECL_TRAILING_REQUIRES,
  VSC_VAR_DECL_INIT,
  VSC_FUNCTION_DECL_BODY,
  VSC_FIELD_DECL_BIT_WIDTH,
  VSC_FIELD_DECL_INIT,
  VSC_ENUM_CONSTANT_DECL,
  VSC_FILE_SCOPE_ASM_DECL_STRING,
  VSC_TEMPLATE_DECL_REQUIRES_CLAUSE,

  // ---- Context is a TypeLoc ----
  VSC_TYPE_OF_TYPE,
  VSC_DECLTYPE_TYPE,
  VSC_ARRAY_TYPE_SIZE,

  // ---- Context is a Stmt ----
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
  VSC_CXX_CONSTRUCT_EXPR,              // also CXXTemporaryObjectExpr
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
  VSC_CONSTANT_EXPR,
  VSC_EXPLICIT_CAST_EXPR,              // ExplicitCastExpr subclasses
  VSC_IMPLICIT_CAST_EXPR,
  VSC_CALL_EXPR_CALLEE,                // CallExpr and subclasses
  VSC_CALL_EXPR_ARG,
  VSC_MEMBER_EXPR,
  VSC_PAREN_EXPR,
  VSC_UNARY_EXPR_OR_TYPE_TRAIT_EXPR,
  VSC_UNARY_OPERATOR,

  // ---- Other contexts ----
  VSC_TEMPLATE_ARGUMENT,

  NUM_VISIT_STMT_CONTEXTS
};

// Return a string like "VSC_NONE", or "unknown" if 'vsc' is invalid.
char const *toString(VisitStmtContext vsc);


// Possible roles for a syntactic type.
enum VisitTypeContext {
  VTC_NONE,

  // ---- Context is a Decl ----
  VTC_DECLARATOR_DECL,
  VTC_TYPEDEF_NAME_DECL,
  VTC_ENUM_DECL_UNDERLYING,
  VTC_CLASS_TEMPLATE_SPECIALIZATION_DECL,
  VTC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL,
  VTC_CXX_RECORD_DECL_BASE,
  VTC_CXX_CTOR_INITIALIZER,
  VTC_FRIEND_DECL,
  VTC_FRIEND_TEMPLATE_DECL,

  // ---- Context is a TypeLoc ----
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

  // ---- Context is a Stmt ----
  VTC_CXX_NEW_EXPR,
  VTC_CXX_PSEUDO_DESTRUCTOR_EXPR_SCOPE,
  VTC_CXX_PSEUDO_DESTRUCTOR_EXPR_DESTROYED,
  VTC_CXX_SCALAR_VALUE_INIT_EXPR,
  VTC_CXX_TYPEID_EXPR,
  VTC_CXX_UNRESOLVED_CONSTRUCT_EXPR,
  VTC_CXX_UUIDOF_EXPR,
  VTC_CXX_TEMPORARY_OBJECT_EXPR,
  VTC_EXPLICIT_CAST_EXPR,
  VTC_UNARY_EXPR_OR_TYPE_TRAIT_EXPR,

  // ---- Other contexts ----
  VTC_TEMPLATE_ARGUMENT,
  VTC_NESTED_NAME_SPECIFIER,
  VTC_DECLARATION_NAME,

  NUM_VISIT_TYPE_CONTEXTS
};

// Return a string like "VTC_NONE", or "unknown" if 'vtc' is invalid.
char const *toString(VisitTypeContext vtc);


// Contexts for a template argument.
enum VisitTemplateArgumentContext {
  VTAC_NONE,

  // ---- Context is a Decl ----
  VTAC_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION_DECL,
  VTAC_CLASS_SCOPE_FUNCTION_SPECIALIZATION_DECL,

  // ---- Context is a TypeLoc ----
  VTAC_TEMPLATE_SPECIALIZATION_TYPE,

  // ---- Context is a Stmt ----
  VTAC_CXX_DEPENDENT_SCOPE_MEMBER_EXPR,
  VTAC_DECL_REF_EXPR,
  VTAC_MEMBER_EXPR,

  NUM_VISIT_TEMPLATE_ARGUMENT_CONTEXTS
};

// Return a string like "VTAC_NONE", or "unknown" if 'vtac' is invalid.
char const *toString(VisitTemplateArgumentContext vtac);


// Context for a nested name specifier (a name scope qualifier).
enum VisitNestedNameSpecifierContext {
  VNNSC_NONE,

  // ---- Context is a Decl ----
  VNNSC_DECLARATOR_DECL,
  VNNSC_ENUM_DECL,
  VNNSC_RECORD_DECL,
  VNNSC_USING_DECL,

  // ---- Context is a TypeLoc ----
  VNNSC_ELABORATED_TYPE,

  // ---- Context is a Stmt ----
  VNNSC_CXX_PSEUDO_DESTRUCTOR_EXPR,
  VNNSC_DECL_REF_EXPR,
  VNNSC_MEMBER_EXPR,

  NUM_VISIT_NESTED_NAME_SPECIFIER_CONTEXTS
};

// Return a string like "VNNS_NONE", or "unknown" if 'vnnsc' is invalid.
char const *toString(VisitNestedNameSpecifierContext vnnsc);


// Context for a DeclarationName.
enum VisitDeclarationNameContext {
  VDNC_NONE,

  // ---- Context is a Decl ----
  VDNC_FUNCTION_DECL,

  // ---- Context is a Stmt ----
  VDNC_DECL_REF_EXPR,

  NUM_VISIT_DECLARATION_NAME_CONTEXTS
};

// Return a string like "VDNC_NONE", or "unknown" if 'vdnc' is invalid.
char const *toString(VisitDeclarationNameContext vdnc);


/*
  Visitor for the clang AST.

  While conceptually similar to RecursiveASTVisitor (RAV), this visitor
  has several advantages:

  1. It is easy to do both pre-order and post-order processing at
     the same time.  Among other things, this makes it possible to
     "push" and "pop" state as the traversal descends and ascends.
     (This is also possible with RAV, but idiosyncratic and somewhat
     unintuitive; see the rav-printer-visitor module for an example.)

  2. The compile-time cost for clients is greatly reduced because the
     interface only requires forward declarations of the relevant AST
     nodes, and the client then need only include definitions of the
     nodes they use.

  3. The set of functions the client can override is much simpler and
     easier to understand.  Instead of the pantheon of TraverseXXX,
     WalkUpXXX, VisitXXX, and ad-hoc extensions like
     dataTraverseStmtPre, everything is done by a small, regular set of
     core visit methods.

  4. Rather than having a separate visitor method for every subclass,
     clients are expected to use 'dyn_cast' as appropriate.  Whether
     this is truly an advantage is of course debatable, but it does
     reduce the size of the interface.  It also arguably makes "visitor"
     a misnomer since the GOF patterns book defines "visitor" as the
     type-dependent dispatch mechanism rather than the tree traversal
     mechanism, but I think the traversal is the more important part.

  5. The core visit methods accept a 'context' parameter that allows
     clients to know the syntactic context in which a node appears,
     which can make some filtering tasks easier.  It's already been
     useful during testing to deal with cases where RAV (to which I am
     comparing) does something unexpected or outright wrong.

  6. The interface uses 'const' pointers instead of non-const.  Neither
     works in every situation, but I think 'const' works more often, at
     least for my intended applications, and of course the visitor
     itself does not make any modifications.

  7. RAV traverses into the semantic Type hierarchy, which IMO confuses
     syntactic exploration with semantic concerns.  In contrast, this
     visitor sticks to traversing the syntactic TypeLoc hierarchy.

  One disadvantage is possibly slower run-time speed due to using
  virtual function calls instead of CRTP static overriding.

  NOTE: Since the 'visit' methods are both what the client overrides,
  and the traversal mechanism, a client that overrides one must always
  arrange to call the base class method when recursion is desired.  But
  as a consequential benefit, when recursion is not desired, or should
  happen at a specific point, the client has direct control over that.
*/
class ClangASTVisitor {
public:      // methods
  // -------- Core visitors --------
  //
  // These visitors form the visitor core, as each corresponds to one of
  // the fundamental concepts in the clang AST, and (by default)
  // recursively traverses child AST nodes.

  // Default: Visit children of 'decl'.
  //
  // Precondition: decl != nullptr.
  //
  virtual void visitDecl(
    VisitDeclContext context,
    clang::Decl const *decl);

  // Default: Visit children of 'decl'.
  //
  // Note that Expr is a subclass of Stmt, so visiting expressions is
  // done with 'visitStmt'.
  //
  // Precondition: stmt != nullptr.
  //
  virtual void visitStmt(
    VisitStmtContext context,
    clang::Stmt const *stmt);

  // Default: Visit children of 'typeLoc'.
  //
  // In the Clang AST, 'TypeLoc' is a particular syntactic description
  // of a type, whereas 'Type' is the thing it semantically denotes.  In
  // an AST traversal, we only traverse the former.
  //
  // Note: RecursiveASTVisitor has a quirk in that it omits calling
  // 'VisitTypeLoc' for the case of a 'QualifiedTypeLoc' (see the
  // comments on 'RecursiveASTVisitor::TraverseQualifiedTypeLoc').  If
  // the client of this interface wants to achieve the same effect, it
  // should check for 'typeLoc' being a 'QualifiedTypeLoc' and skip
  // processing in that case.
  //
  // Precondition: !typeLoc.isNull()
  //
  virtual void visitTypeLoc(
    VisitTypeContext context,
    clang::TypeLoc typeLoc);

  // TODO: visitAttr.

  // Default: Visit the children of 'tal'.
  //
  // Precondition: !tal.getArgument().isNull()
  //
  virtual void visitTemplateArgumentLoc(
    VisitTemplateArgumentContext context,
    clang::TemplateArgumentLoc tal);

  // Default: Visit the children of 'nnsl' by first visiting the prefix
  // of 'nnsl' if it exists, then its final component.
  //
  // Precondition: nnsl.hasQualifier()
  //
  virtual void visitNestedNameSpecifierLoc(
    VisitNestedNameSpecifierContext context,
    clang::NestedNameSpecifierLoc nnsl);

  // Default: If 'dni' is a type name, visit it.  Otherwise do nothing.
  virtual void visitDeclarationNameInfo(
    VisitDeclarationNameContext context,
    clang::DeclarationNameInfo dni);

  // -------- Auxiliary visitors --------
  //
  // By default, these iterate over their children, but are meant to be
  // overridden by clients if needed, especially to skip them.

  // Visit the instantiations of 'ftd'.
  //
  // In order to avoid duplicate visitation, the default visitor will
  // only call this when 'ftd' is canonical, but that is not a
  // precondition.
  //
  // Default: Call 'visitDecl' on each instantiation.
  virtual void visitFunctionTemplateInstantiations(
    clang::FunctionTemplateDecl const *ftd);

  // Visit the instantiations of 'ctd'.
  //
  // The default visitor will only call this when 'ctd' is canonical.
  //
  // Note that this includes instantations of partial specializations;
  // those are found (and visited) as specializations of the primary,
  // even though they are instanitated from the partial specialization.
  //
  // Default: Call 'visitDecl' on each instantiation.
  virtual void visitClassTemplateInstantiations(
    clang::ClassTemplateDecl const *ctd);

  // TODO: Instantiations of variable templates and type alias
  // templates.

  // -------- Leaf visitors --------
  //
  // These are called when certain elements of interest are encountered
  // that the client might care about, but by default they do nothing.

  // This happens when the AST would normally have a TypeLoc, but the
  // type is implicit, like the name of a constructor of a class that
  // otherwise does not have one.  Since the programmer never wrote out
  // the type, there cannot be a TypeLoc for it.  Instead, the AST
  // records only a QualType.  The visitor invokes this method only when
  // there is no TypeLoc.
  //
  // Default: Do nothing.
  virtual void visitImplicitQualType(VisitTypeContext context,
                                     clang::QualType qualType);

  // -------- Helpers --------
  //
  // These functions are used to traverse special kinds of elements.
  // They encapsulate any iteration or other non-trivial processing that
  // the core visitor would otherwise do, so that a client that
  // overrides a core visitor and needs to customize handling for a
  // certain node type does not need to repeat much of what the
  // underlying visitor already does.
  //
  // They are public to facilitate reuse, but are not meant to be
  // overridden by clients, hence not 'virtual'.

  // Visit a nullable Decl.
  void visitDeclOpt(VisitDeclContext context,
                    clang::Decl const * NULLABLE decl)
  {
    if (decl) {
      visitDecl(context, decl);
    }
  }

  // Visit a nullable Stmt.
  void visitStmtOpt(VisitStmtContext context,
                    clang::Stmt const * NULLABLE stmt)
  {
    if (stmt) {
      visitStmt(context, stmt);
    }
  }

  // Assert that 'tsi' is not nullptr, then visit its 'TypeLoc'.
  void visitTypeSourceInfo(VisitTypeContext context,
                           clang::TypeSourceInfo const *tsi);

  // Visit 'tsi' if it is not nullptr.
  void visitTypeSourceInfoOpt(
    VisitTypeContext context,
    clang::TypeSourceInfo const * NULLABLE tsi);

  // Call 'visitDecl' on all of the child declarations.
  void visitNonFunctionDeclContext(
    VisitDeclContext context,
    clang::DeclContext const *dc);

  // Visit the parameters of 'fd'.
  //
  // This is only done for implicit function declarations, since
  // explicit declarations have a TypeLoc with the parameters.
  //
  void visitImplicitFunctionDeclParameters(
    clang::FunctionDecl const *fd);

  // Visit the bases of 'crd'.
  void visitCXXRecordBases(
    clang::CXXRecordDecl const *crd);

  // Visit the TypeLoc in 'base'.
  void visitBaseSpecifier(
    clang::CXXBaseSpecifier const &base);

  // Visit the member initializers in 'ccd'.
  void visitCtorInitializers(
    clang::CXXConstructorDecl const *ccd);

  // If 'init' initializes a base class or is a call to a sibling ctor,
  // then visit the associated TypeLoc.  Otherwise, do nothing.
  void visitCtorInitializer(
    clang::CXXCtorInitializer const *init);

  // In 'tparams', visit the parameters, then the requires clause if
  // present.
  void visitTemplateDeclParameterList(
    clang::TemplateParameterList const *tparams);

  // Visit all of 'args'.  It can be nullptr only if 'numArgs' is 0.
  void visitTemplateArgumentLocArray(
    VisitTemplateArgumentContext context,
    clang::TemplateArgumentLoc const * NULLABLE args,
    unsigned numArgs);

  // Visit the arguments in 'argListInfo'.
  void visitASTTemplateArgumentListInfo(
    VisitTemplateArgumentContext context,
    clang::ASTTemplateArgumentListInfo const *argListInfo);

  // Visit 'argListInfo' if it is not nullptr.
  void visitASTTemplateArgumentListInfoOpt(
    VisitTemplateArgumentContext context,
    clang::ASTTemplateArgumentListInfo const * NULLABLE argListInfo);

  // Visit the parameters in 'ftl'.
  void visitFunctionTypeLocParameters(
    clang::FunctionTypeLoc ftl);

  // Visit the template arguments in 'tstl'.
  void visitTemplateSpecializationTypeLocArguments(
    clang::TemplateSpecializationTypeLoc tstl);

  // Visit the handlers in 'stmt', but *not* the 'try' block, since the
  // default 'visitStmt' does that part itself.
  void visitCXXTryStmtHandlers(
    clang::CXXTryStmt const *stmt);

  // Visit all the statements in 'compound'.
  void visitCompoundStmtBody(
    clang::CompoundStmt const *compound);

  // Visit all of the declarations in 'declStmt'.
  void visitDeclStmtDecls(
    clang::DeclStmt const *declStmt);

  // Visit all of the arguments in 'cexpr'.
  void visitCXXConstructExprArgs(
    clang::CXXConstructExpr const *cexpr);

  // Visit 'nnsl' if it 'hasQualifier()'.
  void visitNestedNameSpecifierLocOpt(
    VisitNestedNameSpecifierContext context,
    clang::NestedNameSpecifierLoc nnsl);

  // Visit the final component of 'nnsl', ignoring its prefix.
  //
  // Precondition: nnsl.hasQualifier()
  //
  void visitNestedNameSpecifierLocFinalComponent(
    clang::NestedNameSpecifierLoc nnsl);

  // Visit the argument expressions in 'callExpr'.
  void visitCallExprArgs(
    clang::CallExpr const *callExpr);

  /* If 'ftd' is the canonical declaration for its template, then visit
     its instantiations.  Otherwise do nothing.

     Restricting to the canonical declaration is important for avoiding
     duplicate visitation when there are multiple declarations.  At
     first it might seem that the same could be done by restricting to
     the definition, but that does not work for the case of a class
     template primary that is never defined, but for which a partial
     specialization *is* defined and instantiated, since we visit
     instantiations of partial specializations as part of the primary.

     However, I do not simply make the "if canonical" variants be the
     "auxiliary visitors" (with 'virtual' specifier) because I do not
     want clients to have to override a method whose name and behavior
     reflects a concern that is primarily internal to the visitor
     mechanism.
  */
  void visitFunctionTemplateInstantiationsIfCanonical(
    clang::FunctionTemplateDecl const *ftd);

  // Visit the instantiations of 'ctd' if it is canonical.
  void visitClassTemplateInstantiationsIfCanonical(
    clang::ClassTemplateDecl const *ctd);

  // Visit the "outer" template parameters associated with 'dd'.  These
  // occur when 'dd' is a declaration of a member of a class template
  // outside the body of its containing class.
  void visitDeclaratorDeclOuterTemplateParameters(
    clang::DeclaratorDecl const *dd);

  // Likewise for a TagDecl.
  void visitTagDeclOuterTemplateParameters(
    clang::TagDecl const *td);

  // Visit the placement args in 'newExpr' if there are any.
  void visitCXXNewExprPlacementArgs(
    clang::CXXNewExpr const *newExpr);

  // Visit the args in 'constructExpr'.
  void visitCXXUnresolvedConstructExprArgs(
    clang::CXXUnresolvedConstructExpr const *constructExpr);
};


#endif // CLANG_AST_VISITOR_H
