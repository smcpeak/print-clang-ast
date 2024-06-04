// pca-command-line-options-test.cc
// Tests for pca-command-line-options.

#include "pca-command-line-options.h"            // module under test

// smbase
#include "smbase/string-util.h"                  // doubleQuote

// libc++
#include <cstdlib>                               // std::exit
#include <iostream>                              // std::cerr, etc.
#include <string>                                // std::string

// libc
#include <assert.h>                              // assert


using std::cerr;
using std::string;


static void test_parseCommandLine()
{
  PCACommandLineOptions options;

  char const *argv[] = {
    "prog",
    "--print-ast-nodes",
    "--no-ast-field-qualifiers",
  };
  int argIndex = 1;
  string err = options.parseCommandLine(argIndex, 3, argv);

  assert(err.empty());
  assert(options.m_printASTNodes == true);
  assert(options.m_noASTFieldQualifiers == true);
}



static void tppsfc1(
  char const *contents,
  char const *expectError,
  char const *expectArgs)
{
  PCACommandLineOptions options;

  string actualError =
    options.parsePrimarySourceFileContents("file", contents);
  if (actualError != expectError) {
    cerr << "parsePrimarySourceFileContents test failed:\n"
         << "  contents: " << doubleQuote(contents) << "\n"
         << "  expectError: " << expectError << "\n"
         << "  actualError: " << actualError << "\n"
         ;
    std::exit(2);
  }

  string actualArgs = options.getAsArgumentsString();
  if (actualArgs != expectArgs) {
    cerr << "parsePrimarySourceFileContents test failed:\n"
         << "  contents: " << doubleQuote(contents)
         << "  expectArgs: " << expectArgs << "\n"
         << "  actualArgs: " << actualArgs << "\n"
         ;
    std::exit(2);
  }
}


static void test_parsePrimarySourceFileContents()
{
  tppsfc1("", "", "");

  tppsfc1("PRINT_CLANG_AST_OPTIONS: --no-ast-field-qualifiers",
          "", "--no-ast-field-qualifiers");

  tppsfc1("\nPRINT_CLANG_AST_OPTIONS: --no-ast-field-qualifiers",
          "", "--no-ast-field-qualifiers");

  tppsfc1("\nPRINT_CLANG_AST_OPTIONS: --no-ast-field-qualifiers\nblah",
          "", "--no-ast-field-qualifiers");

  tppsfc1("\nPRINT_CLANG_AST_OPTIONS:\n--no-ast-field-qualifiers",
          "", "");

  tppsfc1("\nPRINT_CLANG_AST_OPTIONS: --unrecog",
          "file:2:26: unrecognized argument: \"--unrecog\"", "");
}


void pca_command_line_options_unit_tests()
{
  test_parseCommandLine();
  test_parsePrimarySourceFileContents();
}


// EOF
