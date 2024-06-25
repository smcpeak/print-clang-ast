// symbolic-line-mapper-test.cc
// Tests for `symbolic-line-mapper`.

#include "symbolic-line-mapper.h"      // module under test

#include "clang-ast.h"                 // ClangASTUtilTempFile

#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ


OPEN_ANONYMOUS_NAMESPACE


void testOneSymLineColStr(
  SymbolicLineMapper &slm,
  int line,
  char const *expect)
{
  clang::SourceLocation loc = slm.getMainFileLoc(line, 1);
  std::string actual = slm.symLineColStr(loc);
  EXPECT_EQ(actual, expect);
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

  testOneSymLineColStr(slm, 1, "1:1");
  testOneSymLineColStr(slm, 2, "2:1");
  testOneSymLineColStr(slm, 3, "three:1");
  testOneSymLineColStr(slm, 4, "4:1");
  testOneSymLineColStr(slm, 5, "five:1");
  testOneSymLineColStr(slm, 6, "6:1");
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from print-clang-ast.cc.
void symbolic_line_mapper_unit_tests()
{
  testSymLineColStr();
}


// EOF
