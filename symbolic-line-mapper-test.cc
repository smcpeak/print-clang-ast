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


CLOSE_ANONYMOUS_NAMESPACE


// Called from print-clang-ast.cc.
void symbolic_line_mapper_unit_tests()
{
  testSymLineColStr();
}


// EOF
