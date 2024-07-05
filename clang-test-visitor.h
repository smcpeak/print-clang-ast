// clang-test-visitor.h
// `ClangTestVisitor`, a visitor for use in unit tests.

#ifndef CLANG_TEST_VISITOR_H
#define CLANG_TEST_VISITOR_H

#include "clang-ast-visitor.h"         // ClangASTVisitor
#include "clang-util.h"                // ClangUtil

#include "smbase/gdvalue.h"            // gdv::GDValue


// Visitor for use in tests to accumulate the result of calling some
// function(s) on various AST nodes.
class ClangTestVisitor : public ClangASTVisitor, public ClangUtil {
public:      // data
  // Actual results of the calls.
  gdv::GDValue m_actual;

public:      // methods
  ClangTestVisitor(clang::ASTContext &astContext);

  // Scan the TU referred to by `astContext` and then check that
  // `m_actual` equals `expect`.
  void scanTUExpect(gdv::GDValue const &expect);

  // The client then overrides `visitXXX` and populates `m_actual` in
  // the overriders.
};


#endif // CLANG_TEST_VISITOR_H
