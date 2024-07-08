// clang-ast-visitor-nc-test.cc
// Tests for `clang-ast-visitor-nc`.

#include "clang-ast-visitor-nc.h"      // module under test

#include "clang-ast-visitor.h"         // ClangASTVisitor
#include "clang-ast.h"                 // ClangASTUtil

// Must come before sm-test.h.
#include "smbase/vector-util.h"        // operator<<(std::vector)

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <vector>                      // std::vector


OPEN_ANONYMOUS_NAMESPACE


// Record the nodes visited by `ClangASTVisitor`.
class RecordVisitedNodesC : public ClangASTVisitor {
public:      // data
  // Every visited node, in order.
  std::vector<void const *> m_nodes;

public:
  // ClangASTVisitor methods.
  virtual void visitDecl(
    VisitDeclContext context,
    clang::Decl const *decl) override
  {
    m_nodes.push_back(decl);
    ClangASTVisitor::visitDecl(context, decl);
  }

  virtual void visitStmt(
    VisitStmtContext context,
    clang::Stmt const *stmt) override
  {
    m_nodes.push_back(stmt);
    ClangASTVisitor::visitStmt(context, stmt);
  }
};


// Record the nodes visited by `ClangASTVisitorNC`.
class RecordVisitedNodesNC : public ClangASTVisitorNC {
public:      // data
  // Every visited node, in order.
  std::vector<void const *> m_nodes;

public:
  // ClangASTVisitorNC methods.
  virtual void visitDeclNC(
    VisitDeclContext context,
    clang::Decl *decl) override
  {
    m_nodes.push_back(decl);
    ClangASTVisitorNC::visitDeclNC(context, decl);
  }

  virtual void visitStmtNC(
    VisitStmtContext context,
    clang::Stmt *stmt) override
  {
    m_nodes.push_back(stmt);
    ClangASTVisitorNC::visitStmtNC(context, stmt);
  }
};


void compareToConst()
{
  // Just some file to test with.
  ClangASTUtil ast({"in/src/ct-cont-ft-inst.cc"});

  // Run both visitors.
  RecordVisitedNodesC visitorC;
  RecordVisitedNodesNC visitorNC;
  visitorC.scanTU(ast.getASTContext());
  visitorNC.scanTU(ast.getASTContext());

  // They should agree.
  EXPECT_EQ(visitorC.m_nodes, visitorNC.m_nodes);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from pca-unit-tests.cc.
void clang_ast_visitor_nc_unit_tests()
{
  compareToConst();
}


// EOF
