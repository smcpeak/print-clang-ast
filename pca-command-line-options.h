// pca-command-line-options.h
// PCACommandLineOptions class.

#ifndef PCA_COMMAND_LINE_OPTIONS_H
#define PCA_COMMAND_LINE_OPTIONS_H

#include <string>                                // std::string
#include <vector>                                // std::vector


// Class to hold the options that can be set on the header analysis
// command line, and to parse those options out of argv and also out of
// the primary source file.
class PCACommandLineOptions {
public:      // data
  // Use the def file to declare the fields.
  #define BOOL_OPTION(fieldName, defaultValue, optionName, helpText) \
    bool fieldName;
  #include "pca-command-line-options.def"

  // This only exists to be initialized by the ctor after everything
  // else in order to tolerate a trailing comma.
  int m_dummy;

public:      // methods
  PCACommandLineOptions();
  ~PCACommandLineOptions();

  // Print the usage message to stdout.
  static void printUsage(char const *progName);

  // Process a single command line argument.  Return false if it is not
  // recognized.
  bool processArgument(std::string const &arg);

  // Parse options out of 'argv' and into 'this', updating 'argIndex' to
  // point at the first option that was not recognized.  On success,
  // return "".  On error, return an error message, which does not have
  // a newline.
  std::string parseCommandLine(
    int /*IN/OUT*/ &argIndex, int argc, char const **argv);

  // Look through 'contents' to see if it has HA opions, and if so,
  // update 'this'.  On error, return an error message.  'fname' is
  // only used as part of an error message.
  std::string parsePrimarySourceFileContents(
    std::string const &fname, std::string const &contents);

  // Read 'fname', then pass it to 'parsePrimarySourceFileContents'.  On
  // error, return an error message.
  std::string parsePrimarySourceFile(std::string const &fname);

  // Express the options configuration as a sequence of command line
  // arguments.
  std::vector<std::string> getAsArguments() const;

  // Get options as a space-separated string.
  std::string getAsArgumentsString() const;
};


// Unit tests, defined in pca-command-line-options-test.cc.
void pca_command_line_options_unit_tests();

#endif // PCA_COMMAND_LINE_OPTIONS_H
