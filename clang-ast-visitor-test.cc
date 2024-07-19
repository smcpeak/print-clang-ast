// clang-ast-visitor-test.cc
// Tests for `clang-ast-visitor`.

#include "clang-ast-visitor.h"         // module under test

#include "clang-util-ast-visitor.h"    // ClangUtilASTVisitor

#include "smbase/container-util.h"     // smbase::contains
#include "smbase/exc.h"                // EXN_CONTEXT
#include "smbase/save-restore.h"       // SAVE_RESTORE
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/set-util.h"           // setInsert
#include "smbase/xassert.h"            // xassert

#include "clang/AST/Decl.h"            // clang::NamedDecl
#include "clang/Basic/LLVM.h"          // clang::{dyn_cast, isa}

using namespace smbase;

using clang::dyn_cast;
using clang::isa;


OPEN_ANONYMOUS_NAMESPACE


INIT_TRACE("clang-ast-visitor-test");


// Although `ClangASTVisitor` is the module under test, I inherit
// `ClangUtilASTVisitor` to get the `ClangUtil` capabilities too.
//
// The main point of this test is to validate certain visitor
// constraints, such as lack of duplicate visitation.  However, at the
// moment there are a number of places those constraints are violated,
// so all we're doing is exercising it without really checking anything.
//
// TODO: Enable the checks.
//
class ClangASTVisitorTest : public ClangUtilASTVisitor {
public:      // types
  using Base = ClangUtilASTVisitor;

public:      // data
  // True if we are inside `LambdaExpr`.
  bool m_inLambdaExpr;

  // True if we are inside a `FunctionTemplateDecl` corresponding to a
  // deduction guide.
  bool m_inDeductionGuideTemplate;

  // Set of Decls we have visited.
  std::set<clang::Decl const *> m_visitedDecls;

  // Set of Stmts we have visited.
  std::set<clang::Stmt const *> m_visitedStmts;

public:      // methods
  ClangASTVisitorTest(clang::ASTContext &context)
    : ClangUtilASTVisitor(context),
      m_inLambdaExpr(false),
      m_inDeductionGuideTemplate(false),
      m_visitedDecls(),
      m_visitedStmts()
  {}

  virtual void visitDecl(
    VisitDeclContext context,
    clang::Decl const *decl) override
  {
    SAVE_RESTORE(m_inDeductionGuideTemplate);
    if (isDeductionGuideTemplate(decl)) {
      m_inDeductionGuideTemplate = true;
    }

    // We should not visit the same Decl more than once.
    if (!setInsert(m_visitedDecls, decl)) {
      if (m_inLambdaExpr && isa<clang::ParmVarDecl>(decl)) {
        // Lambdas lead to doubly-visited parameters.  Allow it and do
        // not recurse.  Ex: in/src/nested-function.cc.
        return;
      }

      if (m_inDeductionGuideTemplate) {
        // The template parameters get duplicated onto implicit
        // deduction guides.  Ex: in/src/using-type-deduced-targs.cc.
        return;
      }

      // TODO: Add more specific exceptions.
      return;

      xfailure_stringbc("Double visit decl: " << declKindAtLocStr(decl));
    }

    Base::visitDecl(context, decl);
  }

  virtual void visitStmt(
    VisitStmtContext context,
    clang::Stmt const *stmt) override
  {
    SAVE_RESTORE(m_inLambdaExpr);
    if (isa<clang::LambdaExpr>(stmt)) {
      m_inLambdaExpr = true;
    }

    // Statements should also be visited exactly once.
    if (!setInsert(m_visitedStmts, stmt)) {
      // TODO: Add more specific exceptions.
      return;

      xfailure_stringbc("Double visit stmt: " << stmtKindLocStr(stmt));
    }

    Base::visitStmt(context, stmt);
  }
};


CLOSE_ANONYMOUS_NAMESPACE


void clangASTVisitorTest(clang::ASTContext &astContext)
{
  ClangASTVisitorTest visitor(astContext);

  {
    EXN_CONTEXT("visitor.scanTU");
    visitor.scanTU();
  }

  TRACE1_EXPR(visitor.m_visitedDecls.size());
  TRACE1_EXPR(visitor.m_visitedStmts.size());
}


// EOF
