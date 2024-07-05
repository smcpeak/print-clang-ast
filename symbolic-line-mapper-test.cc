// symbolic-line-mapper-test.cc
// Tests for `symbolic-line-mapper`.

#include "symbolic-line-mapper.h"      // module under test

#include "clang-ast.h"                 // ClangASTUtilTempFile
#include "clang-test-visitor.h"        // ClangTestVisitor

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace gdv;

using clang::dyn_cast;


OPEN_ANONYMOUS_NAMESPACE


void testOneSymLineColStr(
  SymbolicLineMapper &slm,
  int line,
  char const *expectLineCol,
  char const *expectLine)
{
  clang::SourceLocation loc = slm.getMainFileLoc(line, 1);

  // symLineColStr
  {
    std::string actual = slm.symLineColStr(loc);
    EXPECT_EQ(actual, expectLineCol);
  }

  // symLineStr
  {
    std::string actual = slm.symLineStr(loc);
    EXPECT_EQ(actual, expectLine);
  }
}


void testSymLineColStr()
{
  ClangASTUtilTempFile ast(
    R"(int x;
      int y;
      /* SYMLINE(three) */
      // 4
      // SYMLINE(five)
      // 6
)");

  SymbolicLineMapper slm(ast.getASTContext());

  testOneSymLineColStr(slm, 1, "1:1",     "1");
  testOneSymLineColStr(slm, 2, "2:1",     "2");
  testOneSymLineColStr(slm, 3, "three:1", "three");
  testOneSymLineColStr(slm, 4, "4:1",     "4");
  testOneSymLineColStr(slm, 5, "five:1",  "five");
  testOneSymLineColStr(slm, 6, "6:1",     "6");
}


// Accumulate result of getting symbolic lines from all DeclRefExprs.
class DeclRefExprVisitor : public ClangTestVisitor {
public:      // data
  // Line mapper to use.
  SymbolicLineMapper m_slm;

public:      // methods
  DeclRefExprVisitor(clang::ASTContext &astContext)
    : ClangTestVisitor(astContext),
      m_slm(astContext)
  {}

  // ClangTestVisitor methods
  virtual void visitStmt(
    VisitStmtContext context,
    clang::Stmt const *stmt) override
  {
    if (auto declRefExpr = dyn_cast<clang::DeclRefExpr>(stmt)) {
      clang::SourceLocation loc = declRefExpr->getLocation();
      clang::SourceLocation spellingLoc = m_srcMgr.getSpellingLoc(loc);
      clang::SourceLocation expansionLoc = m_srcMgr.getExpansionLoc(loc);

      m_actual.setInsert(GDVTuple{
        namedDeclStr(declRefExpr->getDecl()),
        #if 0 // for debugging
          locStr(loc),
          locStr(spellingLoc),
          locStr(expansionLoc),
        #endif
        m_slm.symLineStr(loc),
        m_slm.symLineStr(spellingLoc),
        m_slm.symLineStr(expansionLoc),
      });
    }

    ClangTestVisitor::visitStmt(context, stmt);
  }
};


void testWithMacroExpansion()
{
  ClangASTUtil ast({"in/src/macro-expansion.cc"});
  DeclRefExprVisitor(ast.getASTContext()).scanTUExpect(GDVSet{
    GDVTuple{
      "hiddenFunction()",
      "expansionLine",
      "macroDefnLine",
      "expansionLine",
    },
    GDVTuple{
      "x",
      "returnLine",
      "returnLine",
      "returnLine",
    },
  });
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from print-clang-ast.cc.
void symbolic_line_mapper_unit_tests()
{
  testSymLineColStr();
  testWithMacroExpansion();
}


// EOF
