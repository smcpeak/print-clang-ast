// util.cc
// Code for util.h.

#include "util.h"                      // this module

#include <cstdlib>                     // std::exit
#include <cstring>                     // std::strchr
#include <iostream>                    // std::{cerr, endl}

#include <assert.h>                    // assert

using std::string;


void die(char const *reason)
{
  std::cout.flush();

  std::cerr << reason << std::endl;
  std::exit(4);
}


void die(std::string const &reason)
{
  die(reason.c_str());
}


void die_fileLine(
  string const &msg,
  char const *sourceFile,
  int sourceLine)
{
  die_fileLine(msg.c_str(), sourceFile, sourceLine);
}

void die_fileLine(
  char const *msg,
  char const *sourceFile,
  int sourceLine)
{
  die(stringb("\n" << sourceFile << ":" << sourceLine << ": " << msg));
}


bool startsWith(string const &full, string const &prefix)
{
  // Use the idiom of reverse search from the beginning, which just
  // looks in that one place.
  return full.rfind(prefix, 0 /*pos*/) == 0;
}


bool endsWith(string const &full, string const &suffix)
{
  if (full.size() >= suffix.size()) {
    return full.find(suffix, full.size() - suffix.size() /*pos*/) !=
           string::npos;
  }
  else {
    return false;
  }
}


std::string removeSuffix(std::string const &full,
                         std::string const &suffix)
{
  if (endsWith(full, suffix)) {
    return full.substr(0, full.size() - suffix.size());
  }
  else {
    return full;
  }
}


bool hasSubstring(string const &haystack, string const &needle)
{
  return haystack.find(needle) != string::npos;
}


string replaceAll(string const &orig,
                  string const &from, string const &to)
{
  string ret(orig);
  string::size_type i = ret.find(from);
  while (i != string::npos) {
    ret.replace(i, from.size(), to);
    i = ret.find(from, i + to.size());
  }
  return ret;
}

string padTo(string const &s, size_t len)
{
  if (s.size() < len) {
    return s + string(len - s.size(), ' ');
  }
  else {
    return s;
  }
}


char const * const cWhitespaceChars = " \t\n\r\f\v";


// https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
string trimWhitespace(string s)
{
  // Trim from start.
  s.erase(0, s.find_first_not_of(cWhitespaceChars));

  // Trim from end.
  s.erase(s.find_last_not_of(cWhitespaceChars) + 1);

  return s;
}


bool isCWhitespace(char c)
{
  return std::strchr(cWhitespaceChars, c) != nullptr;
}


std::string doubleQuote(std::string const &s)
{
  std::ostringstream os;
  os << '"';

  for (char ch : s) {
    unsigned c = static_cast<unsigned char>(ch);    // in [0,255]
    switch (c) {
      case '\t': os << "\\t"; break;
      case '\n': os << "\\n"; break;
      case '\r': os << "\\r"; break;
      case '\f': os << "\\f"; break;
      case '\v': os << "\\v"; break;
      case '\"': os << "\\\""; break;
      case '\\': os << "\\\\"; break;

      default:
        if (c < 32 || c > 126) {
          // Use three digit octal escape because hex isn't safe.
          os << '\\' << std::oct;
          os.width(3);
          os.fill('0');
          os << c;
        }
        else {
          os << ch;
        }
        break;
    }
  }

  os << '"';
  return os.str();
}


std::string removeIndentation(std::string const &text)
{
  // Walk 'text' to find newlines and count their following spaces.
  int minIndent = -1;
  for (string::size_type i = text.find('\n');
       i != string::npos;
       i = text.find('\n', i)) {
    // Count spaces here.
    int indent = 0;
    ++i;
    while (i < text.size() && text[i] == ' ') {
      ++i;
      ++indent;
    }

    // Fold 'indent' into 'minIndent'.
    if (minIndent == -1 || indent < minIndent) {
      minIndent = indent;
    }
  }

  // Bail early if there is no indentation to remove.
  if (minIndent < 1) {
    return text;
  }

  // Construct a new string by removing 'minIndent' spaces after every
  // newline.
  std::ostringstream oss;
  for (string::size_type i=0; i < text.size(); ++i) {
    oss << text[i];
    if (text[i] == '\n') {
      // Skip 'minIndent' characters.
      for (int j=0; j < minIndent; ++j) {
        ++i;
      }
    }
  }

  return oss.str();
}


std::string addIndentation(std::string const &text,
                           std::string const &indent)
{
  return replaceAll(text, "\n", string("\n") + indent);
}


// Given that we start with 'text[delimPos]=='('', advance 'delimPos'
// until it is one character past the ')' that matches the original '('.
// Return false if no such close-paren is found.
static bool skipBalancedParens(string const &text,
                               string::size_type &delimPos)
{
  assert(text[delimPos] == '(');

  int parenNesting = 1;
  ++delimPos;

  while (parenNesting > 0) {
    delimPos = text.find_first_of("()", delimPos);
    if (delimPos == string::npos) {
      // Unbalanced parens, so malformed.
      return false;
    }

    if (text[delimPos] == '(') {
      ++parenNesting;
    }
    else {
      --parenNesting;
    }

    ++delimPos;
  }

  // We have now seen as many '(' as ')', and 'delimPos' points at
  // the character after the last ')'.
  assert(parenNesting == 0);
  return true;
}


std::string removeDefaultArguments(std::string text, bool angleBrackets)
{
  // We have processed everything in 'text' up to 'cursor'.
  string::size_type cursor = 0;

  // What counts as a delimiter?
  char const * const delimiters = (angleBrackets? ",()<>" : ",()");

  while (cursor < text.size()) {
    // Find the first '='.
    string::size_type eqPos = text.find('=', cursor);
    if (eqPos == string::npos) {
      break;
    }

    // If it's immediately followed by another '=', then this is a
    // comparison, not a default argument.
    if (eqPos+1 < text.size() &&
        text[eqPos+1] == '=') {
      cursor = eqPos+2;
      continue;
    }

    // If it immediately follows certain characters, then it is the
    // second character of a comparison or a compound assignment.
    if (eqPos > 0 &&
        std::strchr("<>!|^&+-", text[eqPos-1])) {
      cursor = eqPos+1;
      continue;
    }

    // Find the delimiter that ends the default argument.
    string::size_type delimPos = eqPos;
    while (true) {
      // Search forward to find the next delimiter.
      delimPos = text.find_first_of(delimiters, delimPos);
      if (delimPos == string::npos) {
        // No delimiter.  This could be the pure virtual "=0" after the
        // main signature.  Keep the whole thing.
        return text;
      }

      // Skip past balanced parentheses.
      if (text[delimPos] == '(') {
        if (!skipBalancedParens(text, delimPos)) {
          return text;       // malformed
        }

        // Return to the loop start to look for the next delimiter.
        continue;
      }

      // Skip past balanced angle brackets outside any parens.
      if (text[delimPos] == '<') {
        int angleNesting = 1;
        ++delimPos;

        while (angleNesting > 0) {
          delimPos = text.find_first_of("(<>", delimPos);
          if (delimPos == string::npos) {
            // Unbalanced angles, so malformed.
            return text;
          }

          if (text[delimPos] == '(') {
            if (!skipBalancedParens(text, delimPos)) {
              return text;   // malformed
            }
          }
          else if (text[delimPos] == '<') {
            ++angleNesting;
            ++delimPos;
          }
          else {
            --angleNesting;
            ++delimPos;
          }
        }

        // We have seen as many '<' as '>'.
        assert(angleNesting == 0);
        continue;
      }

      // The found delimiter ends the argument.
      break;
    }

    // Search backward to collect all whitespace before '='.
    while (eqPos > 0 && isCWhitespace(text[eqPos-1])) {
      --eqPos;
    }

    // Remove the default argument.
    text.erase(eqPos, delimPos-eqPos);

    // Advance the cursor to the point just past where the delimiter is
    // after the erasure.
    cursor = eqPos+1;
  }

  return text;
}


std::string removeDefaultTemplateArguments(std::string text)
{
  return removeDefaultArguments(text, true /*angleBrackets*/);
}


std::string trimCppCommentText(std::string comment)
{
  assert(comment.size() >= 2 &&
         comment[0] == '/' &&
         comment[1] == '/');

  return trimWhitespace(comment.substr(2));
}


std::string trimCCommentText(std::string comment)
{
  assert(comment.size() >= 4 &&
         comment[0] == '/' &&
         comment[1] == '*' &&
         comment[comment.size()-2] == '*' &&
         comment[comment.size()-1] == '/');

  return trimWhitespace(comment.substr(2, comment.size()-4));
}


template <class CONTAINER>
string genericCommaSeparate(CONTAINER const &strings,
                            char const *sep)
{
  std::ostringstream oss;

  int ct=0;
  for (string const &s : strings) {
    if (ct++ > 0) {
      oss << sep;
    }
    oss << s;
  }

  return oss.str();
}


string commaSeparate(std::set<string> const &strings,
                     char const *sep)
{
  return genericCommaSeparate(strings, sep);
}


string commaSeparate(std::vector<string> const &strings,
                     char const *sep)
{
  return genericCommaSeparate(strings, sep);
}


std::string joinWithPrefixes(std::set<std::string> const &strings,
                             char const *prefix)
{
  std::ostringstream oss;

  for (string const &s : strings) {
    oss << prefix << s;
  }

  return oss.str();
}


std::string pathFinalName(std::string const &path)
{
  auto i = path.rfind('/');
  if (i != string::npos) {
    return path.substr(i+1);
  }
  else {
    return path;
  }
}


string bracesSetIfMultiple(std::set<string> const &strings)
{
  if (strings.size() == 1) {
    auto it = strings.begin();
    return *it;
  }
  else {
    return stringb("{" << commaSeparate(strings) << "}");
  }
}


// EOF
