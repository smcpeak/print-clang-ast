// pca-util.h
// Some general-purpose utilities.

#ifndef PCA_UTIL_H
#define PCA_UTIL_H

// smbase
#include "smbase/sm-pp-util.h"         // SM_PP_CAT

// libc++
#include <cassert>                     // assert
#include <cstddef>                     // std::size_t
#include <map>                         // std::map
#include <set>                         // std::set
#include <sstream>                     // std::ostringstream
#include <string>                      // std::string
#include <utility>                     // std::make_pair
#include <vector>                      // std::vector


// 2024-06-04: Removed `stringb` in favor of the one in
// `smbase/stringb.h`.


// Print a message and exit.
void die(char const *reason);
void die(std::string const &reason);

// Same, but with an associated source file and line.
void die_fileLine(
  std::string const &msg,
  char const *sourceFile,
  int sourceLine);
void die_fileLine(
  char const *msg,
  char const *sourceFile,
  int sourceLine);


// If 'full' ends with 'suffix', return a string like 'full' but with
// that suffix removed.  Otherwise just return 'full'.
std::string removeSuffix(std::string const &full,
                         std::string const &suffix);

// True if 'needle' is a substring of 'haystack'.
bool hasSubstring(std::string const &haystack, std::string const &needle);


// Return 'orig' with all occurrences of 'from' replaced with 'to'.
// Occurrences are scanned left to right, with the subsequent scan
// starting right after the substituted text.
std::string replaceAll(std::string const &orig,
                       std::string const &from, std::string const &to);


// Add spaces to the end of 's' to ensure its length is at least
// 'len'.
std::string padTo(std::string const &s, std::size_t len);



// String containing C whitespace characters: " \t\n\r\f\v".
extern char const * const cWhitespaceChars;

// 2024-06-04: Removed `trimWhitespace` in favor of the one in
// `smbase/string-util.h`.

// True if 'c' is considered whitespace in C.
bool isCWhitespace(char c);


// 2024-06-04: Removed `doubleQuote` in favor of the one in
// `smbase/string-util`.


// If 's' contains newlines, and they are all followed by at least one
// space, then consistently removed spaces after newlines until at least
// one newline is no longer followed by a space, and return the result.
// Otherwise, return 's' unchanged.
//
// Note: Spaces at the start of the string are preserved, and do not
// play a role in how much indentation is removed elsewhere.
std::string removeIndentation(std::string const &text);


// Add 'indent' after every newline in 'text'.
std::string addIndentation(std::string const &text,
                           std::string const &indent);

// Write to 'os' 'indentLevel*2' spaces, then return 'os'.
std::ostream &indentPrefix(std::ostream &os, int indentLevel);


// Given 'text', which has the identifier and parameters of a function
// definition, remove all default arguments.  If 'angleBrackets', then
// 'text' is a template parameter list delimited by angle brackets.
std::string removeDefaultArguments(std::string text,
                                   bool angleBrackets = false);

// Call 'removeDefaultArguments' with 'angleBrackets=true'.
std::string removeDefaultTemplateArguments(std::string text);


// Given the text of a C++ comment, including the leading "//", remove
// that leading syntax, plus any whitespace after it, and any whitespace
// at the end.
std::string trimCppCommentText(std::string comment);

// Given the text of a C comment, starting with "/*" and ending with
// "*/", remove the comment delimiters and then remove any leading and
// trailing whitespace from what remains.
std::string trimCCommentText(std::string comment);


// Join all of the 'strings' with 'sep'.
std::string commaSeparate(std::set<std::string> const &strings,
                          char const *sep = ", ");
std::string commaSeparate(std::vector<std::string> const &strings,
                          char const *sep = ", ");


// Join all of 'strings' with each preceded by 'prefix', including the
// first.
std::string joinWithPrefixes(std::set<std::string> const &strings,
                             char const *prefix);


// If 'path' contains at least one directory separator, return
// everything after that last separator.  Otherwise, return the whole
// string.
std::string pathFinalName(std::string const &path);


// If 'strings' has multiple (or zero) elements, yield a
// brace-enclosed, comma-separated list of them.  Otherwise just yield
// the element by itself.
std::string bracesSetIfMultiple(std::set<std::string> const &strings);


// 2024-06-04: Removed `splitNonEmpty`.  It is defined in
// `smbase/string-util.h`.


/* 2024-06-04: Removed in favor of `smbase/save-restore.h`:

    SaveRestore
    SAVE_RESTORE
    SetRestore
    SET_RESTORE
*/


// Dereference 'p' after asserting it is not nullptr.
template <class T>
T const &assertDeref(T const *p)
{
  assert(p);
  return *p;
}

template <class T>
T &assertDeref(T *p)
{
  assert(p);
  return *p;
}


// Unit tests, defined in pca-util-test.cc.  Aborts on failure.
void pca_util_unit_tests();


#endif // PCA_UTIL_H
