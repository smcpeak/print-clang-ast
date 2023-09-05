// stringref-parse.cc
// Code for stringref-parse.h.

#include "stringref-parse.h"                     // this module

#include "util.h"                                // isCWhitespace

#include <cstring>                               // std::strlen
#include <string>                                // std::string

#include <assert.h>                              // assert


using std::string;


StringRefParse::StringRefParse(
  llvm::StringRef const &text,
  unsigned cursor,
  unsigned lowerBound)
  : StringRefParse(text, cursor, lowerBound, text.size())
{}


StringRefParse::StringRefParse(
  llvm::StringRef const &text,
  unsigned cursor,
  unsigned lowerBound,
  unsigned upperBound)
  : m_text(text),
    m_cursor(cursor),
    m_lowerBound(lowerBound),
    m_upperBound(upperBound)
{
  assertInvariants();
}


StringRefParse StringRefParse::operator= (StringRefParse const &obj)
{
  if (this != &obj) {
    assert(&m_text == &(obj.m_text));
    m_cursor = obj.m_cursor;
    m_lowerBound = obj.m_lowerBound;
    m_upperBound = obj.m_upperBound;
  }
  return *this;
}


void StringRefParse::setCursorAndBounds(
  unsigned cursor, unsigned lowerBound, unsigned upperBound)
{
  m_cursor = cursor;
  m_lowerBound = lowerBound;
  m_upperBound = upperBound;

  assertInvariants();
}


void StringRefParse::setCursor(unsigned cursor)
{
  m_cursor = cursor;
  assertInvariants();
}


void StringRefParse::setCursorClamp(unsigned cursor)
{
  if (cursor < m_lowerBound) {
    m_cursor = m_lowerBound;
  }
  else if (cursor > m_upperBound) {
    m_cursor = m_upperBound;
  }
  else {
    m_cursor = cursor;
  }

  assertInvariants();
}


void StringRefParse::setCursorAndLowerBound(unsigned cursor)
{
  m_lowerBound = m_cursor = cursor;
  assertInvariants();
}


void StringRefParse::setUpperBoundToCursor()
{
  m_upperBound = m_cursor;
  assertInvariants();
}


void StringRefParse::advancePastBlankLinesAfterToken()
{
  // Seek forward to the next whitespace.
  while (m_cursor < m_upperBound &&
         !isCWhitespace(m_text[m_cursor])) {
    ++m_cursor;
  }

  // Don't back up into the token characters if we do not find a
  // newline.
  unsigned localLowerBound = m_cursor;

  // Now seek to the next non-whitespace.
  skipWS();

  // Back up to right after the preceding newline.
  while (m_cursor > localLowerBound &&
         m_text[m_cursor-1] != '\n') {
    --m_cursor;
  }

  assertInvariants();
}


bool StringRefParse::backupToCppCommentStart()
{
  unsigned o = m_cursor;

  while (o >= m_lowerBound+2 &&
         !(m_text[o-1]=='/' && m_text[o-2]=='/') &&
         m_text[o-1]!='\n') {
    --o;
  }

  if (o >= m_lowerBound+2 &&
      (m_text[o-1]=='/' && m_text[o-2]=='/')) {
    // We found the start of a comment.
    m_cursor = o-2;
    assertInvariants();
    return true;
  }

  // We did not find it.
  return false;
}


bool StringRefParse::onPPDirectiveLine() const
{
  unsigned cursor = m_cursor;

  // Search backward for a newline or hash character.
  while (cursor > m_lowerBound &&
         m_text[cursor-1] != '\n' &&
         m_text[cursor-1] != '#') {
    --cursor;
  }

  if (cursor > m_lowerBound &&
      m_text[cursor-1] == '#') {
    // We found a hash before finding a newline, so say yes.
    return true;
  }
  else {
    return false;
  }
}


void StringRefParse::advancePastNextNL()
{
  while (m_cursor < m_upperBound &&
         m_text[m_cursor] != '\n') {
    ++m_cursor;
  }

  if (m_cursor < m_upperBound &&
      m_text[m_cursor] == '\n') {
    ++m_cursor;
  }

  assertInvariants();
}


void StringRefParse::backupToWSStart()
{
  // Back up once to get into the WS area, if there is any.
  if (m_cursor > m_lowerBound && isCWhitespace(m_text[m_cursor-1])) {
    --m_cursor;
  }
  else {
    return;
  }

  while (true) {
    // Back up to the start of the WS.
    while (m_cursor > m_lowerBound && isCWhitespace(m_text[m_cursor-1])) {
      --m_cursor;
    }

    if (backupToCppCommentStart()) {
      // Keep going backward.
      continue;
    }
    else {
      // We are not at the end of a comment, so stop.
      break;
    }
  }

  // If we stopped on a preprocessor directive, go to the next line.
  if (onPPDirectiveLine()) {
    advancePastNextNL();
  }

  assertInvariants();
}


void StringRefParse::backupToLineStart()
{
  while (m_cursor > m_lowerBound &&
         m_text[m_cursor-1] != '\n') {
    --m_cursor;
  }
  assertInvariants();
}


void StringRefParse::backup(unsigned amt)
{
  for (unsigned i=0; i<amt && m_cursor>m_lowerBound; ++i) {
    --m_cursor;
  }
  assertInvariants();
}


void StringRefParse::advancePastContiguousIncludes()
{
  // Most recent position that is at the start of a line after an
  // #include directive.  (This is simply assumed to be true at the
  // start.)
  unsigned afterRecentIncludeLine = m_cursor;

  bool advanced = true;
  while (m_cursor < m_upperBound && advanced) {
    advanced = false;

    skipWS();

    advanced |= skipCCommentIf();
    advanced |= skipCppCommentIf();
    if (skipIncludeIf()) {
      afterRecentIncludeLine = m_cursor;
      advanced = true;
    }
  }

  // We get here when whatever is at 'm_cursor' is not whitespace, a
  // comment, nor an #include.  Return to just after the most recent
  // #include.
  m_cursor = afterRecentIncludeLine;

  assertInvariants();
}


bool StringRefParse::skipWS()
{
  bool ret = false;

  while (m_cursor < m_upperBound &&
         isCWhitespace(m_text[m_cursor])) {
    ++m_cursor;
    ret = true;
  }

  assertInvariants();
  return ret;
}


bool StringRefParse::skipNonWS()
{
  bool ret = false;

  while (m_cursor < m_upperBound &&
         !isCWhitespace(m_text[m_cursor])) {
    ++m_cursor;
    ret = true;
  }

  assertInvariants();
  return ret;
}


bool StringRefParse::skipCCommentIf()
{
  if (lookingAt("/*")) {
    m_cursor += 2;
    while (m_cursor < m_upperBound &&
           !lookingAt("*/")) {
      m_cursor += 1;
    }
    if (lookingAt("*/")) {
      m_cursor += 2;
    }

    assertInvariants();
    return true;
  }
  else {
    return false;
  }
}


bool StringRefParse::skipCppCommentIf()
{
  if (lookingAt("//")) {
    advancePastNextNL();
    return true;
  }
  else {
    return false;
  }
}


bool StringRefParse::skipIncludeIf()
{
  if (lookingAt("#include")) {
    advancePastNextNL();
    return true;
  }
  else {
    return false;
  }
}


bool StringRefParse::skipStringIf(char const *prefix)
{
  if (lookingAt(prefix)) {
    m_cursor += std::strlen(prefix);
    assertInvariants();
    return true;
  }
  else {
    return false;
  }
}


bool StringRefParse::lookingAt(char const *prefix) const
{
  unsigned i = 0;
  while (m_cursor+i < m_upperBound &&
         prefix[i] != '\0') {
    if (m_text[m_cursor+i] != prefix[i]) {
      return false;
    }
    ++i;
  }

  return prefix[i] == '\0';
}


bool StringRefParse::searchFor(char const *searchString)
{
  llvm::StringRef searchStringRef(searchString);

  // Create a bounded view to avoid going beyond the upper bound.
  llvm::StringRef bounded(m_text.data(), m_upperBound);

  // Search forward starting at the cursor.
  size_t idx = bounded.find(searchStringRef, m_cursor);

  if (idx != string::npos) {
    // Found it.
    m_cursor = idx + searchStringRef.size();
    assertInvariants();
    return true;
  }
  else {
    return false;
  }
}


std::string StringRefParse::getNextWSSeparatedToken()
{
  skipWS();

  if (m_cursor >= m_upperBound) {
    // No token.
    assert(m_cursor == m_upperBound);
    return "";
  }

  // We are on a non-WS character.
  unsigned tokenStart = getCursor();

  // Skip past the token.
  skipNonWS();
  unsigned tokenEnd = getCursor();
  assert(tokenEnd > tokenStart);

  return string(m_text.data() + tokenStart, tokenEnd-tokenStart);
}


void StringRefParse::getLineCol(
  int /*OUT*/ &line, int /*OUT*/ &col) const
{
  line = 1;
  col = 1;

  for (unsigned i = m_lowerBound; i < m_cursor; ++i) {
    if (m_text[i] == '\n') {
      ++line;
      col = 1;
    }
    else {
      ++col;
    }
  }
}


std::string StringRefParse::getLineColStr() const
{
  int line, col;
  getLineCol(line, col);
  return stringb(line << ':' << col);
}


void StringRefParse::assertInvariants() const
{
  assert(m_upperBound <= m_text.size());
  assert(m_lowerBound <= m_cursor);
  assert(                m_cursor <= m_upperBound);
}


// EOF
