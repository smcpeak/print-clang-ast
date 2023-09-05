// pca-command-line-options.cc
// Code for pca-command-line-options.h.

#include "pca-command-line-options.h"            // this module

#include "file-util.h"                           // readFile
#include "stringref-parse.h"                     // StringRefParse
#include "util.h"                                // startsWith, commaSeparate

#include <cstdlib>                               // std::exit
#include <iostream>                              // std::cout, etc.
#include <string>                                // std::string

using std::cout;
using std::string;


PCACommandLineOptions::PCACommandLineOptions()
  :
    // Use the def file to specify default values.
    #define BOOL_OPTION(fieldName, defaultValue, optionName, helpText) \
      fieldName(defaultValue),
    #include "pca-command-line-options.def"

    m_dummy(0)
{}


PCACommandLineOptions::~PCACommandLineOptions()
{}


/*static*/ void PCACommandLineOptions::printUsage(char const *progName)
{
  cout << "usage: " << progName << " [<pca-options>] <compiler-options>\n";

  cout << R"""(
This program prints out some information about the Clang AST produced
by compiling a single source file.

<pca-options>:

)""";

  // Use the def file to emit --help text.
  #define BOOL_OPTION(fieldName, defaultValue, optionName, helpText) \
    cout << "  " << optionName << "\n\n"                             \
         << "    " << helpText << "\n\n";
  #include "pca-command-line-options.def"

  cout << R"""(Any option that is not among those listed above will be interpreted as
the first <compiler-option>.
)""";
}


bool PCACommandLineOptions::processArgument(std::string const &arg)
{
  // Allow all cases to begin with 'else'.
  if (false) {}

  // Use the def file to process arguments.
  #define BOOL_OPTION(fieldName, defaultValue, optionName, helpText) \
    else if (arg == (optionName)) {                                  \
      fieldName = !(defaultValue);                                   \
    }
  #include "pca-command-line-options.def"

  else {
    // Argument unrecognized.
    return false;
  }

  return true;
}


std::string PCACommandLineOptions::parseCommandLine(
  int /*IN/OUT*/ &argIndex, int argc, char const **argv)
{
  for (; argIndex < argc; ++argIndex) {
    string arg = argv[argIndex];

    if (!processArgument(arg)) {
      // End of recognized options for the header analysis.
      break;
    }
  }

  // At the moment, I don't do any diagnosis of command lines here,
  // since unrecognized options are treated as the end of options, so
  // the return value is always "".  But if I were to validate
  // something, then my plan would be to return an error message.
  return "";
}


std::string PCACommandLineOptions::parsePrimarySourceFileContents(
  std::string const &fname,
  std::string const &contents)
{
  // Set up a parsing cursor.
  StringRefParse cursor(contents);

  // Scan for some options to parse.
  if (cursor.searchFor("PRINT_CLANG_AST_OPTIONS: ")) {
    unsigned argsStart = cursor;

    // Find the next newline and set that as the stop point.
    cursor.advancePastNextNL();
    cursor.setUpperBoundToCursor();

    // Go back.
    cursor = argsStart;

    // Pull out whitespace-separated arguments.
    while (true) {
      string arg = cursor.getNextWSSeparatedToken();
      if (arg.empty()) {
        break;
      }

      if (!processArgument(arg)) {
        cursor.backup(arg.size());
        return stringb(fname << ":" << cursor.getLineColStr() <<
                       ": unrecognized argument: " <<
                       doubleQuote(arg));
      }
    }
  }

  return "";
}


std::string PCACommandLineOptions::parsePrimarySourceFile(
  std::string const &fname)
{
  // Read the file.
  string contents;
  string err = readFile(contents, fname);
  if (!err.empty()) {
    return err;
  }

  // Parse it.
  return parsePrimarySourceFileContents(fname, contents);
}


std::vector<std::string> PCACommandLineOptions::getAsArguments() const
{
  std::vector<string> args;

  // Use the def file to map field values to options.
  #define BOOL_OPTION(fieldName, defaultValue, optionName, helpText) \
    if (fieldName != (defaultValue)) {                               \
      args.push_back(optionName);                                    \
    }
  #include "pca-command-line-options.def"

  return args;
}


std::string PCACommandLineOptions::getAsArgumentsString() const
{
  return commaSeparate(getAsArguments(), " ");
}


// EOF
