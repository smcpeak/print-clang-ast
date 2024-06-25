// stringref-parse-test.cc
// Tests for stringref-parse.

#include "stringref-parse.h"                     // module under test

#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ
#include "smbase/stringb.h"                      // stringb

#include <cstdlib>                               // std::exit
#include <functional>                            // std::function
#include <iostream>                              // std::cerr
#include <string>                                // std::string


using std::string;
using std::cerr;


OPEN_ANONYMOUS_NAMESPACE


// The 'testCase' contains two markers, "[S]" indicating where the
// cursor should start, and "[E]" indicating where it should end.  Both
// markers are removed before actually performing the action.
template <class RESULT>
void testOneGeneric(
  char const *testCase,
  char const *actionLabel,
  RESULT expectResult,
  std::function<RESULT (StringRefParse &)> action)
{
  string text(testCase);

  string::size_type start = text.find("[S]");
  assert(start != string::npos);

  string::size_type end = text.find("[E]");
  assert(end != string::npos);

  if (start < end) {
    text.erase(end, 3);

    text.erase(start, 3);
    end -= 3;
  }

  else {
    text.erase(start, 3);

    text.erase(end, 3);
    start -= 3;
  }

  // Set up the object to perform the action.
  StringRefParse parse(text, start);

  // Do it.
  RESULT actualResult = action(parse);

  // Check that we got what was expected.
  if (!( parse.getCursor() == end &&
         actualResult == expectResult )) {
    cerr << "test failed: " << actionLabel << ":\n"
            "  expectCursor: " << end << "\n"
            "  actualCursor: " << parse.getCursor() << "\n"
            "  expectResult: " << expectResult << "\n"
            "  actualResult: " << actualResult << "\n"
            "  testcase:\n" << testCase << "\n"
            ;
    std::exit(2);
  }
}


// I don't know why, but the template won't automatically be used for
// bool results.
void testOne(
  char const *testCase,
  char const *actionLabel,
  bool expectResult,
  std::function<bool (StringRefParse &)> action)
{
  testOneGeneric<bool>(testCase, actionLabel, expectResult, action);
}


// Run 'testOne', then run it again with a newline prepended to
// 'testCase', expecting the same result.
template <class RESULT>
void testOneGeneric_prependNL(
  char const *testCase,
  char const *actionLabel,
  RESULT expectResult,
  std::function<RESULT (StringRefParse &)> action)
{
  testOneGeneric(testCase, actionLabel, expectResult, action);

  testOneGeneric(stringb("\n" << testCase).c_str(),
                 actionLabel, expectResult, action);
}


void testOne_prependNL(
  char const *testCase,
  char const *actionLabel,
  bool expectResult,
  std::function<bool (StringRefParse &)> action)
{
  testOneGeneric_prependNL<bool>(testCase, actionLabel,
                                 expectResult, action);
}


void test_advancePastBlankLinesAfterToken()
{
  static char const * const tests[] = {
    // Degenerate case.
    "[S][E]",

    // Normal cases.
    "[S]token\n[E]next",
    "[S]token\n\n[E]next",
    "[S]token\n[E]  next",
    "[S]token\n\n[E]  next",
    "[S]token\n\n  \n[E]  next",

    // No newline after the token.
    "[S]token[E]  ",
  };

  for (auto t : tests) {
    testOne_prependNL(t, "advancePastBlankLinesAfterToken", true,
      [](StringRefParse &p) -> bool {
        p.advancePastBlankLinesAfterToken();
        return true;
      });
  }
}


void test_backupToCppCommentStart()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    // Degenerate.
    { "[S][E]", false },

    // Normal case.
    { "[E]// comment[S]\n", true },

    // Start in the middle of the comment.
    { "[E]// comment[S] text\n", true },

    // No newline.
    { "[E]// comment[S]", true },

    // Not in a comment.
    { "comment[S][E]\n", false },

    // Just one slash.
    { "/ comment[E][S]\n", false },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "backupToCppCommentStart",
                      t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.backupToCppCommentStart();
      });
  }
}


void test_onPPDirectiveLine()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    // Degenerate.
    { "[S][E]", false },

    // Normal case.
    { "#[S][E]include\n", true },

    // If we are right at the start, we say no.  (That is just how the
    // logic currently works.  I don't think there is a particularly
    // good reason for it.)
    { "[S][E]#include\n", false },

    // Start at the end of the line.
    { "#include[S][E]\n", true },

    // No hashes.
    { "include[S][E]\n", false },
    { "[S][E]include\n", false },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "onPPDirectiveLine", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.onPPDirectiveLine();
      });
  }
}


void test_advancePastNextNL()
{
  static char const * const tests[] = {
    // Degenerate.
    "[S][E]",

    // Normal.
    "[S]text\n[E]",

    // Extra newline.
    "[S]text\n[E]\n",

    // No newline.
    "[S]text[E]",
  };

  for (auto t : tests) {
    testOne_prependNL(t, "advancePastNextNL", true,
      [](StringRefParse &p) -> bool {
        p.advancePastNextNL();
        return true;
      });
  }
}


void test_backupToWSStart_tcc_true()
{
  static char const * const tests[] = {
    // Degenerate.
    "[S][E]",

    // Move backward past WS to the string start.  (This shows why we do
    // not use 'testOne_prependNL' here.)
    "[E]\n[S]",

    // Normal.
    "text[E]  [S]",
    "text[E] \n [S]",

    // Back up through a C++ comment.
    "text[E] // comment [S]",

    // But not if the immediately preceding character is non-WS.  (That
    // is just how the logic currently works.)
    "text // comment[E][S]",

    // Through several comment lines.
    "  text[E]\n// comment1\n//comment 2\n[S]",

    // If we would nominally stop on a preprocessor directive, go past
    // its line.
    "#include\n[E]// comment1\n//comment 2\n[S]",
  };

  for (auto t : tests) {
    testOne(t, "backupToWSStart", true,
      [](StringRefParse &p) -> bool {
        p.backupToWSStart(true /*throughCppComments*/);
        return true;
      });
  }
}


void test_backupToWSStart_tcc_false()
{
  static char const * const tests[] = {
    // Degenerate.
    "[S][E]",

    // Do *not* back up through a C++ comment.
    "text // comment[E] [S]",
  };

  for (auto t : tests) {
    testOne(t, "backupToWSStart", true,
      [](StringRefParse &p) -> bool {
        p.backupToWSStart(false /*throughCppComments*/);
        return true;
      });
  }
}


void test_backupToLineStart()
{
  static char const * const tests[] = {
    "[S][E]",
    "[E]  [S]",
    "\n[E]what[S]ever",
  };

  for (auto t : tests) {
    testOne_prependNL(t, "backupToLineStart", true,
      [](StringRefParse &p) -> bool {
        p.backupToLineStart();
        return true;
      });
  }
}


void test_advancePastContiguousIncludes()
{
  static char const * const tests[] = {
    "[S][E]",

    "[S]#include\n"
    "[E]\n",

    "[S]#include\n"
    "#include\n"
    "[E]\n",

    "[S]#include\n"
    "[E]\n"
    "text\n",

    "[S] #include\n"
    "//comment\n"
    "/*comment*/#include\n"
    "[E]\n"
    "\n"
    "  //comment\n"
    "/*comment*/\n"
    "text",

  };

  for (auto t : tests) {
    testOne_prependNL(t, "advancePastContiguousIncludes", true,
      [](StringRefParse &p) -> bool {
        p.advancePastContiguousIncludes();
        return true;
      });
  }
}


void test_skipWS()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", false },
    { "[S][E]x", false },
    { "[S] [E]x", true },
    { "[S] \r\n\t\v\f[E]x", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipWS", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.skipWS();
      });
  }
}


void test_skipNonWS()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", false },
    { "[S]x[E]", true },
    { "[S]x[E] ", true },
    { "[S]xy[E] ", true },
    { "[S][E] x", false },
    { "[S]blah[E]\r\n", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipNonWS", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.skipNonWS();
      });
  }
}


void test_skipCCommentIf()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", false },
    { "[S][E]x", false },
    { "[S][E]/", false },

    // Comment is not immediately at cursor.
    { "[S][E] /**/", false },

    { "[S]/**/[E]x", true },
    { "[S]/*blah*/[E]x", true },
    { "[S]/*bl/*ah*/[E]x", true },
    { "[S]/*bl/*ah**/[E]x", true },
    { "[S]/*bl/*ah*/[E]*/", true },

    // Unterminated.
    { "[S]/*bl/*ah[E]", true },
    { "[S]/*bl/*ah*[E]", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipCCommentIf", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.skipCCommentIf();
      });
  }
}


void test_skipCppCommentIf()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", false },
    { "[S][E]x", false },
    { "[S][E]/", false },

    // Comment is not immediately at cursor.
    { "[S][E] //", false },

    { "[S]//x[E]", true },
    { "[S]//x\n[E]", true },
    { "[S]//x\n[E]x", true },
    { "[S]//xyz\n[E]x", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipCppCommentIf", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.skipCppCommentIf();
      });
  }
}


void test_skipCommentsAndWhitespace()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", false },
    { "[S][E]x", false },
    { "[S][E]/", false },
    { "[S] [E]/", true },

    { "[S] //\n/*blah\nblah*/ [E]x", true },
    { "[S] //\n/*blah\nblah*/ //\n/*blah\nblah*/[E]x", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipCommentsAndWhitespace", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.skipCommentsAndWhitespace();
      });
  }
}


void test_skipIncludeIf()
{
  static struct TestCase {
    char const *m_text;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", false },
    { "[S][E]#includ", false },

    // Not at cursor.
    { "[S][E] #include", false },

    { "[S]#include[E]", true },
    { "[S]#include \"stuff\"\n[E]", true },
    { "[S]#include \"stuff\"\n[E]blah", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipIncludeIf", t.m_expectResult,
      [](StringRefParse &p) -> bool {
        return p.skipIncludeIf();
      });
  }
}


void test_skipStringIf()
{
  static struct TestCase {
    char const *m_text;
    char const *m_prefix;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", "", true },
    { "[S][E]", "x", false },

    { "[S][E]a", "abc", false },
    { "[S][E]ab", "abc", false },
    { "[S]abc[E]", "abc", true },
  };

  for (auto t : tests) {
    testOne_prependNL(t.m_text, "skipStringIf", t.m_expectResult,
      [&](StringRefParse &p) -> bool {
        return p.skipStringIf(t.m_prefix);
      });
  }
}


void test_searchFor()
{
  static struct TestCase {
    char const *m_text;
    char const *m_searchString;
    bool m_expectResult;
  } const tests[] = {
    { "[S][E]", "", true },
    { "[S][E]", "x", false },

    { "[S][E]a", "abc", false },
    { "[S][E]ab", "abc", false },
    { "[S]abc[E]", "abc", true },
    { "[S]abc[E] ", "abc", true },
    { "[S]ababc[E] ", "abc", true },
    { "abc[S]ababc[E] ", "abc", true },
  };

  for (auto t : tests) {
    testOne(t.m_text, "searchFor", t.m_expectResult,
      [&](StringRefParse &p) -> bool {
        return p.searchFor(t.m_searchString);
      });
  }
}


void test_getNextWSSeparatedToken()
{
  static struct TestCase {
    char const *m_text;
    string m_expectResult;
  } const tests[] = {
    { "[S][E]", "" },
    { "[S] [E]", "" },
    { "[S]abc[E]", "abc" },
    { "[S]abc[E] ", "abc" },
    { "[S] abc[E] ", "abc" },
  };

  for (auto t : tests) {
    testOneGeneric_prependNL<string>(t.m_text, "getNextWSSeparatedToken",
      t.m_expectResult,
      [&](StringRefParse &p) -> string {
        return p.getNextWSSeparatedToken();
      });
  }
}


void test_getLineColStr()
{
  static struct TestCase {
    char const *m_text;
    string m_expectResult;
  } const tests[] = {
    { "[S][E]", "1:1" },
    { " [S][E]", "1:2" },
    { " [S][E] ", "1:2" },
    { " \n[S][E] ", "2:1" },
    { " \n [S][E] ", "2:2" },
    { " \n \n [S][E] ", "3:2" },
    { " \n \n   [S][E] ", "3:4" },
  };

  for (auto t : tests) {
    testOneGeneric<string>(t.m_text, "getLineColStr", t.m_expectResult,
      [&](StringRefParse &p) -> string {
        return p.getLineColStr();
      });
  }
}


void test_textUpTo()
{
  StringRefParse p("abcdefghi", 3);

  // Simple case.
  assert(p.textUpTo(6) == "def");

  // end past upper bound.
  assert(p.textUpTo(16) == "defghi");

  // end less than cursor.
  assert(p.textUpTo(0) == "");
}


void test_getNextIdentifier()
{
  StringRefParse p("id123 3abc");

  EXPECT_EQ(p.getNextIdentifier(), "id123");
  EXPECT_EQ(p.getNextIdentifier(), "");
  p.skipWS();
  EXPECT_EQ(p.getNextIdentifier(), "");
  EXPECT_EQ(p.getNextChar(), '3');
  EXPECT_EQ(p.getNextIdentifier(), "abc");
  xassert(!p.hasText());
}


CLOSE_ANONYMOUS_NAMESPACE


void stringref_parse_unit_tests()
{
  test_advancePastBlankLinesAfterToken();
  test_backupToCppCommentStart();
  test_onPPDirectiveLine();
  test_advancePastNextNL();
  test_backupToWSStart_tcc_true();
  test_backupToWSStart_tcc_false();
  test_backupToLineStart();
  test_advancePastContiguousIncludes();
  test_skipWS();
  test_skipNonWS();
  test_skipCCommentIf();
  test_skipCppCommentIf();
  test_skipCommentsAndWhitespace();
  test_skipIncludeIf();
  test_skipStringIf();
  test_searchFor();
  test_getNextWSSeparatedToken();
  test_getLineColStr();
  test_textUpTo();
  test_getNextIdentifier();
}


// EOF
