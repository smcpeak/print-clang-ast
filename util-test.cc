// util-test.cc
// Tests for util module.

#include "util.h"                      // module under test

#include <cstdlib>                     // std::abort
#include <iostream>                    // std::cerr

#include <assert.h>                    // assert


using std::cerr;
using std::string;


template <class T>
T transformForPrinting(T const &t)
{
  return t;
}

std::string transformForPrinting(std::string const &s)
{
  return doubleQuote(s);
}


template <class INPUT, class OUTPUT>
void expectEq(char const *label, INPUT const &input,
              OUTPUT const &expect, OUTPUT const &actual)
{
  if (expect != actual) {
    cerr << "test failed: " << label << ":\n"
         << "  input : " << transformForPrinting(input) << "\n"
         << "  expect: " << transformForPrinting(expect) << "\n"
         << "  actual: " << transformForPrinting(actual) << "\n";
    cerr.flush();
    std::abort();
  }
}


static void tew1(char const *full, char const *suffix, bool expect)
{
  bool actual = endsWith(full, suffix);
  expectEq("endsWith", full, expect, actual);
}

static void test_endsWith()
{
  tew1("", "", true);
  tew1("a", "a", true);
  tew1("a", "b", false);
  tew1("a\n", "\n", true);
}


static void ttw1(char const *input, char const *expect)
{
  string actual = trimWhitespace(input);
  expectEq("trimWhitespace", input, string(expect), actual);
}

static void test_trimWhitespace()
{
  ttw1("", "");
  ttw1("x", "x");
  ttw1(" x", "x");
  ttw1("x ", "x");
  ttw1(" x ", "x");
  ttw1(" x y ", "x y");
  ttw1("  x", "x");
  ttw1("x  ", "x");
  ttw1(" \r\n\f\v\thello \r\n\f\v\t", "hello");
}


static void tdq1(string const &input, char const *expect)
{
  string actual = doubleQuote(input);
  expectEq("doubleQuote", input, string(expect), actual);
}

static void test_doubleQuote()
{
  tdq1("", "\"\"");
  tdq1("hi", "\"hi\"");

  tdq1(string("\x00\x01\x02\x1f \\A~\x7f\x80\x81\xfe\xff", 13),
       "\"\\000\\001\\002\\037 \\\\A~\\177\\200\\201\\376\\377\"");

  tdq1(" \t\n\r\f\v",
       "\" \\t\\n\\r\\f\\v\"");
}


static void tpfn1(char const *input, char const *expect)
{
  string actual = pathFinalName(input);
  expectEq("pathFinalName", input, string(expect), actual);
}

static void test_pathFinalName()
{
  tpfn1("", "");
  tpfn1("a", "a");
  tpfn1("a/b", "b");
  tpfn1("a/", "");
  tpfn1("a/b/c", "c");
}



static void tri1(char const *input, char const *expect)
{
  string actual = removeIndentation(input);
  expectEq("removeIndentation", input, string(expect), actual);
}

static void test_removeIndentation()
{
  // Some simple cases that don't really involve indentation.
  tri1("", "");
  tri1("x", "x");
  tri1(" x", " x");
  tri1("\n", "\n");
  tri1("\n\n", "\n\n");

  // Lines are indented consistently.
  tri1("x\n"
       "  y\n"
       "  z"       ,
       "x\n"
       "y\n"
       "z");

  // First line is indented more.
  tri1("x\n"
       "    y\n"
       "  z"       ,
       "x\n"
       "  y\n"
       "z");

  // Second line is indented more.
  tri1("x\n"
       "  y\n"
       "    z"     ,
       "x\n"
       "y\n"
       "  z");

  // A newline at the end prevents removing indentation.
  tri1("x\n"
       "  y\n"
       "    z\n"
       ""          ,
       "x\n"
       "  y\n"
       "    z\n"
       "");
}


static void trda1(char const *input, string expect)
{
  string actual = removeDefaultArguments(input);
  expectEq("removeDefaultArguments", input, expect, actual);
}

static void trda_noChange(char const *input)
{
  trda1(input, input);
}

static void test_removeDefaultArguments()
{
  trda_noChange("int f()");
  trda_noChange("int f(int x)");

  trda1("int f(int x = 3)", "int f(int x)");
  trda1("int f(int x = 3, int y = 4)", "int f(int x, int y)");
  trda1("int f(int x = 3,\n"
        "      int y = 4)",
        "int f(int x,\n"
        "      int y)");
  trda1("int f(int x =\n"
        "              3,\n"
        "      int y = 4)",
        "int f(int x,\n"
        "      int y)");

  trda_noChange("int f() = 0");

  trda1("int f(int x = 2) = 0", "int f(int x) = 0");

  // This uses '==', not '='.
  trda_noChange("__builtin_strlen(Str) == N - 1");

  // Comparison.
  trda_noChange(R"(
  int method(
    int a,
#if CLANG_VERSION_MAJOR <= 14
    int b_14,
#else
    int b_16,
#endif
    int c,
    int d)
)");
}


static void trdta1(char const *input, string expect)
{
  string actual = removeDefaultTemplateArguments(input);
  expectEq("removeDefaultTemplateArguments", input, expect, actual);
}

static void test_removeDefaultTemplateArguments()
{
  trdta1("template <>",
         "template <>");

  trdta1("template <class T>",
         "template <class T>");

  trdta1("template <class T = int>",
         "template <class T>");

  trdta1("template <class A, class B>",
         "template <class A, class B>");

  trdta1("template <class A, class B = float>",
         "template <class A, class B>");

  trdta1("template <int x, int y>",
         "template <int x, int y>");

  trdta1("template <int x = 2, int y = 3>",
         "template <int x, int y>");

  trdta1("template <int x = (2) + (3), int y = ((3))>",
         "template <int x, int y>");

  trdta1("template <int x = func(1,2), bool y = (3>4)>",
         "template <int x, bool y>");

  trdta1("template <class T, int v = OtherTemplate<T>::value>",
         "template <class T, int v>");

  // From clang/include/llvm/ADT/Optional.h.
  trdta1(
    "template <typename T,\n"
    "          bool = (llvm::is_trivially_copy_constructible<T>::value &&\n"
    "                  std::is_trivially_copy_assignable<T>::value &&\n"
    "                  (llvm::is_trivially_move_constructible<T>::value ||\n"
    "                   !std::is_move_constructible<T>::value) &&\n"
    "                  (std::is_trivially_move_assignable<T>::value ||\n"
    "                   !std::is_move_assignable<T>::value))>\n"
    ,
    "template <typename T,\n"
    "          bool>\n"
  );
}


static void tai1(char const *text, char const *indent, string expect)
{
  string actual = addIndentation(text, indent);
  expectEq("addIndentation", text, expect, actual);
}

static void test_addIndentation()
{
  tai1("", "", "");
  tai1("", "i", "");
  tai1("\n", "i", "\ni");
  tai1("abc\ndef", "  ", "abc\n  def");
  tai1("abc\ndef\nghi", "  ", "abc\n  def\n  ghi");
}


static void test_joinWithPrefixes()
{
  std::set<std::string> strings;
  assert(joinWithPrefixes(strings, "") == "");
  assert(joinWithPrefixes(strings, "x") == "");

  strings.insert("a");
  assert(joinWithPrefixes(strings, "") == "a");
  assert(joinWithPrefixes(strings, "x") == "xa");

  strings.insert("bc");
  assert(joinWithPrefixes(strings, "") == "abc");
  assert(joinWithPrefixes(strings, "x") == "xaxbc");
}


void util_unit_tests()
{
  test_endsWith();
  test_trimWhitespace();
  test_doubleQuote();
  test_pathFinalName();
  test_removeIndentation();
  test_removeDefaultArguments();
  test_removeDefaultTemplateArguments();
  test_addIndentation();
  test_joinWithPrefixes();
}


// EOF
