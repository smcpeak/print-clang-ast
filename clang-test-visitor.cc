// clang-test-visitor.cc
// Code for `clang-test-visitor.h`.

#include "clang-test-visitor.h"        // this module

#include "smbase/exc.h"                // EXN_CONTEXT
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/sm-test.h"            // EXPECT_EQ

using namespace gdv;


ClangTestVisitor::ClangTestVisitor(clang::ASTContext &astContext)
  : ClangASTVisitor(),
    ClangUtil(astContext),
    m_actual(GDVK_SET)
{}


void ClangTestVisitor::scanTUExpect(GDValue const &expect)
{
  EXN_CONTEXT(m_mainFileName);

  ClangASTVisitor::scanTU(getASTContext());
  EXPECT_EQ(m_actual, expect);
}


// EOF
