// clang-util-test.cc
// Tests for `clang-util` module.

#include "clang-util.h"                // module under test

#include "clang-ast-visitor.h"         // ClangASTVisitor
#include "clang-ast.h"                 // ClangASTUtil

#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/temporary-file.h"     // smbase::TemporaryFile

#include "clang/AST/Decl.h"            // clang::NamedDecl

#include <sstream>                     // std::ostringstream

using namespace gdv;
using namespace smbase;

using clang::dyn_cast;


INIT_TRACE("clang-util-test");


OPEN_ANONYMOUS_NAMESPACE


// Visitor that calls `getInstFromDeclOpt` on every `Decl` node and
// accumulates the results.
class GIFDVisitor : public ClangASTVisitor, public ClangUtil {
public:      // types
  using Base = ClangASTVisitor;

public:      // data
  // Actual results of the calls.
  GDValue m_actual;

public:      // methods
  GIFDVisitor(clang::ASTContext &astContext)
    : ClangASTVisitor(),
      ClangUtil(astContext),
      m_actual(GDVK_SEQUENCE)
  {}

  // ClangASTVisitor methods
  virtual void visitDecl(
    VisitDeclContext context,
    clang::Decl const *decl) override;
};


void GIFDVisitor::visitDecl(
  VisitDeclContext context,
  clang::Decl const *decl)
{
  TRACE2("visitDecl: " << declKindAtLocStr(decl));

  if (auto namedDecl = dyn_cast<clang::NamedDecl>(decl)) {
    if (auto instFrom = getInstFromDeclOpt(namedDecl)) {
      m_actual.sequenceAppend(GDVTuple({
        namedDeclCompactIdentifier(namedDecl),
        namedDeclCompactIdentifier(instFrom)
      }));
    }
    else {
      // Do not add anything if the call returns `nullptr`.
    }
  }

  Base::visitDecl(context, decl);
}


void testOneGetInstFromDeclOpt(char const *source, GDValue const &expect)
{
  TemporaryFile temp("gifd", "cc", source);
  ClangASTUtil ast({temp.getFname()});

  GIFDVisitor visitor(ast.getASTContext());
  visitor.scanTU(ast.getASTContext());

  EXPECT_EQ(visitor.m_actual, expect);
}


void testGetInstFromDeclOpt()
{
  testOneGetInstFromDeclOpt(
    R"(
      template <typename T>
      void f() {}

      void g()
      {
        f<int>();
        f<float>();
      }
    )",
    GDValue(GDVSequence{
      GDVTuple{"f<int>()",   "f<T>()"},
      GDVTuple{"f<float>()", "f<T>()"},
    })
  );

  // TODO: More tests.
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from print-clang-ast.cc.
void clang_util_unit_tests()
{
  testGetInstFromDeclOpt();
}


// EOF
