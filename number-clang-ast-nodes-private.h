// number-clang-ast-nodes-private.h
// Private decls for number-clang-ast-nodes module.

#ifndef NUMBER_CLANG_AST_NODES_PRIVATE_H
#define NUMBER_CLANG_AST_NODES_PRIVATE_H

#include "number-clang-ast-nodes.h"              // public decls for this module

#include "clang-util.h"                          // ClangUtil

#include "clang/AST/RecursiveASTVisitor.h"       // clang::RecursiveASTVisitor


// Visitor to assign IDs to nodes.
class NumberClangASTNodes : public ClangUtil,
                            public clang::RecursiveASTVisitor<NumberClangASTNodes> {
public:      // data
  // The numbering we are building.
  ClangASTNodeNumbering &m_numbering;

public:      // methods
  NumberClangASTNodes(clang::ASTContext &astContext,
                      ClangASTNodeNumbering &numbering);

  ~NumberClangASTNodes();

  // Visit everything so the numbering is complete.
  bool shouldVisitTemplateInstantiations() const
    { return true; }
  bool shouldVisitImplicitCode() const
    { return true; }

  // RecursiveASTVisitor methods.
  bool VisitType(clang::Type *type);
  bool VisitDecl(clang::Decl *decl);
  bool VisitStmt(clang::Stmt *stmt);
};


#endif // NUMBER_CLANG_AST_NODES_PRIVATE_H
