// clang-ast-visitor.h
// ClangASTVisitor class.

#ifndef CLANG_AST_VISITOR_H
#define CLANG_AST_VISITOR_H

// this dir
#include "clang-decl-context-fwd.h"              // clang::DeclContext [n]
#include "clang-decl-cxx-fwd.h"                  // clang::CXXBaseSpecifier [n]
#include "clang-decl-template-fwd.h"             // clang::TemplateParameterList [n]
#include "clang-template-base-fwd.h"             // clang::TemplateArgument [n]
#include "clang-type-fwd.h"                      // clang::QualType [n]
#include "clang-type-loc-fwd.h"                  // clang::TypeLoc [n]

// clang
#include "clang/AST/ASTFwd.h"                    // clang::{Stmt, Decl, ...} [n]


// Possible roles a Decl can have in the AST.
enum VisitDeclContext {
  // Used when initiating a traversal from outside the visitor.
  VDC_NONE,

  // Templated decl of a TemplateDecl.
  VDC_TEMPLATE_TEMPLATED,

  // The decl inside a 'friend' decl that is being declared to be a
  // friend.
  VDC_FRIEND_FRIEND_DECL,

  // The friend declared by a template friend declaration.
  VDC_FRIEND_TEMPLATE_FRIEND_DECL,

  // Decl inside a DeclContext.
  //
  // TODO: Split this out according to the specific DeclContext.
  VDC_DECL_CONTEXT_DECL,

  // Instantation of a function template.
  VDC_FUNCTION_TEMPLATE_INSTANTIATION,

  // Instantiation of a class template.
  VDC_CLASS_TEMPLATE_INSTANTIATION,

  // Parameter of a FunctionDecl.
  VDC_FUNCTION_PARAMETER,

  // Parameter of a TemplateDecl.
  VDC_TEMPLATE_PARAMETER,

  NUM_VISIT_DECL_CONTEXTS
};

// Return a string like "VDC_NONE", or "unknown" if 'vdc' is invalid.
char const *toString(VisitDeclContext vdc);


// Possible roles for a Stmt.
enum VisitStmtContext {
  VSC_NONE,

  // Trailing-requires constraint on a declarator.
  VSC_DECLARATOR_TRAILING_REQUIRES,

  // Initializer of a VarDecl.
  VSC_VAR_DECL_INIT,

  // Body of a FunctionDecl.
  VSC_FUNCTION_BODY,

  // Bitfield bit width of a FieldDecl.
  VSC_FIELD_BIT_WIDTH,

  // Initializer of a FieldDecl.
  VSC_FIELD_INIT,

  // Initializer of an EnumConstantDecl.
  VSC_ENUM_CONSTANT_INIT,

  // String of a FileScopeAsmDecl.
  VSC_FILE_SCOPE_ASM_STRING,

  // Requires clause of a TemplateDecl.
  VSC_TEMPLATE_REQUIRES_CLAUSE,

  NUM_VISIT_STMT_CONTEXTS
};

// Return a string like "VSC_NONE", or "unknown" if 'vsc' is invalid.
char const *toString(VisitStmtContext vsc);


// Possible roles for a syntactic type.
enum VisitTypeContext {
  VTC_NONE,

  // Type of a DeclaratorDecl.
  VTC_DECLARATOR_TYPE,

  // Type of a TypedefNameDecl.
  VTC_TYPEDEF_NAME_TYPE,

  // Underlying type of an EnumDecl.
  VTC_ENUM_UNDERLYING,

  // Base class of a CXXRecordDecl.
  VTC_CLASS_BASE,

  // Base class or delegating initializer in a constructor.
  VTC_CTOR_INIT,

  // For a 'friend' declaration that nominates a type, the type.
  VTC_FRIEND_FRIEND_TYPE,

  // For a 'friend' template declaration of a type, the type.
  VTC_FRIEND_TEMPLATE_FRIEND_TYPE,

  NUM_VISIT_TYPE_CONTEXTS
};

// Return a string like "VTC_NONE", or "unknown" if 'vtc' is invalid.
char const *toString(VisitTypeContext vtc);


/*
  Visitor for the clang AST.

  While conceptually similar to RecursiveASTVisitor, this visitor has
  several advantages:

  1. It is possible to do both pre-order and post-order processing at
     the same time.  Among other things, this makes it possible to
     "push" and "pop" state as the traversal descends and ascends.

  2. The compile-time cost for clients is greatly reduced because the
     interface only requires forward declarations of the relevant AST
     nodes.

  3. The interface is generally simpler and easier to understand.
     Instead of Traverse, WalkUp, and Visit, everything is done by
     visit methods.  Additionally, rather than having a separate Visit
     method for every subclass, clients are expected to use 'dyn_cast'
     as appropriate.

  4. The interface uses 'const' pointers instead of non-const.  Neither
     works in every situation, but I think 'const' works more often, at
     least for my intended applications.

  One disadvantage is possibly slower run-time speed due to using
  virtual function calls instead of CRTP static overriding.

  Since the 'visit' methods are both what the client overrides, and the
  traversal mechanism, a client that overrides one must always arrange
  to call the base class method when recursion is desired.
*/
class ClangASTVisitor {
public:      // methods
  // -------- Core visitors --------
  //
  // These visitors form the visitor core, as each corresponds to one of
  // the fundamental concepts in the clang AST, and (by default)
  // recursively traverses child AST nodes.

  // Default: Visit children of 'decl'.
  virtual void visitDecl(VisitDeclContext context, clang::Decl const *decl);

  // Default: Visit children of 'decl'.
  //
  // Note that Expr is a subclass of Stmt, so visiting expressions is
  // done with 'visitStmt'.
  virtual void visitStmt(VisitStmtContext context, clang::Stmt const *stmt);

  // Default: Visit children of 'typeLoc'.
  //
  // In the Clang AST, 'TypeLoc' is a particular syntactic description
  // of a type, whereas 'Type' is the thing it semantically denotes.  In
  // an AST traversal, we only traverse the former.
  virtual void visitTypeLoc(VisitTypeContext context, clang::TypeLoc typeLoc);

  // -------- Leaf visitors --------
  //
  // These are called when certain elements of interest are encountered
  // that the client might care about, but by default they do nothing.

  // This happens when the AST would normally have a TypeLoc, but the
  // type implicit, like that of a constructor of a class that otherwise
  // does not have one.  Since the programmer never wrote out the type,
  // there cannot be a TypeLoc for it.  Instead, the AST records only a
  // QualType.  The visitor invokes this method only when there is no
  // TypeLoc.
  //
  // Default: Do nothing.
  virtual void visitImplicitQualType(VisitTypeContext context,
                                     clang::QualType qualType);

  // -------- Helper visitors --------
  //
  // These visitors are used to traverse special kinds of elements.
  // They serve two purposes:
  //
  // 1. Encapsulate any iteration or other non-trivial processing that
  //    the core visitor would otherwise do, so that a client that
  //    overrides a core visitor and needs to customize handling for a
  //    certain node type does not need to repeat much of what the
  //    underlying visitor already does.
  //
  // 2. Provide a convenient place to hook into the traversal process in
  //    order to skip unwanted elements.

  // Visit a DeclContext other than FunctionDecl.
  //
  // Default: Call 'visitDecl' on all of the child declarations.
  virtual void visitNonFunctionDeclContext(
    clang::DeclContext const *dc);

  // Visit the instantiations of 'ftd'.
  //
  // Default: Call 'visitDecl' on each instantiation.
  virtual void visitFunctionTemplateInstantiations(
    clang::FunctionTemplateDecl const *ftd);

  // Visit the instantiations of 'ctd'.
  //
  // Default: Call 'visitDecl' on each instantiation.
  virtual void visitClassTemplateInstantiations(
    clang::ClassTemplateDecl const *ctd);

  // Default: Visit the parameters of 'fd'.
  virtual void visitFunctionParameters(
    clang::FunctionDecl const *fd);

  // Default: Visit the bases of 'crd'.
  virtual void visitCXXRecordBases(
    clang::CXXRecordDecl const *crd);

  // Default: Visit the TypeLoc in 'base'.
  virtual void visitBaseSpecifier(
    clang::CXXBaseSpecifier const &base);

  // Default: Visit the member initializers in 'ccd'.
  virtual void visitCtorInitializers(
    clang::CXXConstructorDecl const *ccd);

  // Default: If 'init' initializes a base class or is a call to a
  // sibling ctor, then visit the associated TypeLoc.  Otherwise, do
  // nothing.
  virtual void visitCtorInitializer(
    clang::CXXCtorInitializer const *init);

  // Default: In 'tparams', visit the parameters, then the requires
  // clause (if present).
  virtual void visitTemplateParameterList(
    clang::TemplateParameterList const *tparams);

  // Default: Visit the arguments in 'targs'.
  virtual void visitTemplateArgumentList(
    clang::TemplateArgumentList const *targs);

  // Default: For a type argument, visit its TypeLoc.  For a non-type
  // argument, visit its Expr.  For a template template argument,
  // [TODO, maybe nothing?].
  virtual void visitTemplateArgument(
    clang::TemplateArgument const &arg);
};


#endif // CLANG_AST_VISITOR_H
