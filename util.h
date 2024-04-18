// util.h
// Some general-purpose utilities.

#ifndef PCA_UTIL_H
#define PCA_UTIL_H

#include "sm-pp-util.h"                // SM_PP_CAT

#include <cassert>                     // assert
#include <cstddef>                     // std::size_t
#include <map>                         // std::map
#include <set>                         // std::set
#include <sstream>                     // std::ostringstream
#include <string>                      // std::string
#include <utility>                     // std::make_pair
#include <vector>                      // std::vector


// Construct a string in-place using ostream operators.
#define stringb(stuff) \
  (static_cast<std::ostringstream const &>(std::ostringstream() << stuff).str())


// Pseudo-attribute meaning a pointer can be nullptr.
#define NULLABLE /*nullable*/


// Place in a class definition to inhibit the auto-generated copy
// operations.
#define NO_OBJECT_COPIES(name)              \
  name(name&) = delete;                     \
  void operator=(name&) = delete /*user ;*/


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


// For use with libc++ containers.
template <class CONTAINER, class K>
bool contains(CONTAINER const &c, K const &k)
{
  return c.find(k) != c.end();
}


template <class K, class V>
void addIfNotAlready(std::map<K,V> &m, K const &k, V const &v)
{
  if (m.find(k) == m.end()) {
    m.insert(std::make_pair(k,v));
  }
}


// True if 'full' begins with 'prefix'.
bool startsWith(std::string const &full, std::string const &prefix);

// True if 'full' ends with 'suffix'.
bool endsWith(std::string const &full, std::string const &suffix);

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

// Trim whitespace from both ends of 's'.
std::string trimWhitespace(std::string s);

// True if 'c' is considered whitespace in C.
bool isCWhitespace(char c);


/*
  Return 's' enclosed in double quotes, and with special characters
  escaped with C syntax, specifically:

  - non-printing characters escaped using octal,

  - whitespace and metacharacters (backslash and double-quote) escaped
    using backslash mnemonics, and

  - all other characters represent themselves.
*/
std::string doubleQuote(std::string const &s);


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


// Split 'text' into non-empty words separated by 'sep', which never
// appears in any of the result words.
std::vector<std::string> splitNonEmpty(std::string const &text, char sep);


// Restore a variable's value when this object goes out of scope.
template <class T>
class SaveRestore {
public:      // data
  T &m_variable;
  T m_origValue;

public:      // methods
  SaveRestore(T &variable)
    : m_variable(variable),
      m_origValue(variable)
  {}

  ~SaveRestore()
  {
    m_variable = m_origValue;
  }
};


// Set a variable to a value, then restore when going out of scope.
template <class T>
class SetRestore : public SaveRestore<T> {
public:      // methods
  SetRestore(T &variable, T const &newValue)
    : SaveRestore<T>(variable)
  {
    this->m_variable = newValue;
  }
};


// SetRestore with a uniquely-named restorer object and deduced type.
#define SET_RESTORE(variable, value) \
  SetRestore<decltype(variable)> SM_PP_CAT(set_restore_,__LINE__) \
    (variable, value) /* user ; */


// Declare a bunch of a set-like operators for enum types.
//
// This was copied from smbase/sm-macros.h.
#define ENUM_BITWISE_AND(Type)                  \
  inline Type operator& (Type f1, Type f2)      \
    { return (Type)((int)f1 & (int)f2); }       \
  inline Type& operator&= (Type &f1, Type f2)   \
    { return f1 = f1 & f2; }

#define ENUM_BITWISE_OR(Type)                   \
  inline Type operator| (Type f1, Type f2)      \
    { return (Type)((int)f1 | (int)f2); }       \
  inline Type& operator|= (Type &f1, Type f2)   \
    { return f1 = f1 | f2; }

#define ENUM_BITWISE_XOR(Type)                  \
  inline Type operator^ (Type f1, Type f2)      \
    { return (Type)((int)f1 ^ (int)f2); }       \
  inline Type& operator^= (Type &f1, Type f2)   \
    { return f1 = f1 ^ f2; }

#define ENUM_BITWISE_NOT(Type, ALL)             \
  inline Type operator~ (Type f)                \
    { return (Type)((~(int)f) & ALL); }

#define ENUM_BITWISE_OPS(Type, ALL)             \
  ENUM_BITWISE_AND(Type)                        \
  ENUM_BITWISE_OR(Type)                         \
  ENUM_BITWISE_XOR(Type)                        \
  ENUM_BITWISE_NOT(Type, ALL)


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


// Unit tests, defined in util-test.cc.  Aborts on failure.
void util_unit_tests();


#endif // PCA_UTIL_H
