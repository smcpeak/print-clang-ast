// stringref-parse.h
// StringRefParse class.

#ifndef STRINGREF_PARSE_H
#define STRINGREF_PARSE_H

#include "llvm/ADT/StringRef.h"                  // llvm::StringRef


// This class allows ad-hoc parsing within a given StringRef.
class StringRefParse {
private:     // data
  // The text we are parsing.
  llvm::StringRef m_text;

  // Current position.  We consider the "current character" to be the
  // one at 'm_text[m_cursor]'.  But note that the cursor is allowed to
  // point to the end+1 character.
  //
  // Invariant: m_lowerBound <= m_cursor
  // Invariant:                 m_cursor <= m_upperBound
  unsigned m_cursor;

  // Do not move the cursor before this point.
  unsigned m_lowerBound;

  // Do not move the cursor after this point.
  //
  // Invariant: m_upperBound <= m_text.size()
  unsigned m_upperBound;

public:      // methods
  // The upper bound is set to 'text.size()'.
  explicit StringRefParse(
    llvm::StringRef const &text,
    unsigned cursor = 0,
    unsigned lowerBound = 0);

  StringRefParse(
    llvm::StringRef const &text,
    unsigned cursor,
    unsigned lowerBound,
    unsigned upperBound);

  StringRefParse(StringRefParse const &obj) = default;
  StringRefParse& operator= (StringRefParse const &obj) = default;

  // Getters.
  llvm::StringRef getText() const { return m_text; }
  unsigned getLowerBound() const { return m_lowerBound; }
  unsigned getUpperBound() const { return m_upperBound; }
  unsigned getCursor() const { return m_cursor; }

  // True if there is still text to scan.
  bool hasText() const { return m_cursor < m_upperBound; }

  // Get the character at `m_cursor` without advancing.  Requires
  // `hasText()`.
  char peekNextChar() const;

  // Get and advance.  Requires `hasText()`.
  char getNextChar();

  // Set the offsets.  This asserts that the invariants hold.
  void setCursorAndBounds(
    unsigned cursor, unsigned lowerBound, unsigned upperBound);

  // Set the cursor, asserting it is within bounds.
  void setCursor(unsigned cursor);

  // Set the cursor, but clamp it to be within bounds.
  void setCursorClamp(unsigned cursor);

  // Set both the cursor and the lower bound to 'cursor'.
  void setCursorAndLowerBound(unsigned cursor);

  // Set the upper bound to equal the current cursor.
  void setUpperBoundToCursor();

  // Allow the object to be used like an unsigned offset.
  StringRefParse& operator= (unsigned cursor)
    { setCursor(cursor); return *this; }
  operator unsigned () const
    { return getCursor(); }

  // Given that the cursor is pointing at a token, move past that token
  // and the following blank lines.  Stop just after a newline character
  // if possible.
  void advancePastBlankLinesAfterToken();

  // Check if the character at the cursor is part of a C++ comment.  If
  // so, set it to point at the first slash of that comment and return
  // true.  Otherwise, leave it unchanged and return false.
  //
  // BUG: This ignores the possibility of string literals.  I should
  // explore what I can do with clang::Lexer.
  bool backupToCppCommentStart();

  // Return true if cursor is on a line with a preprocessor directive,
  // but the cursor is not at the very start of the line.
  //
  // BUG: This does not handle string literals or C comments properly.
  bool onPPDirectiveLine() const;

  // Move cursor forward to just past the next newline.
  void advancePastNextNL();

  // If the character before the cursor is whitespace, move it backward
  // to the first WS character in the contiguous sequence.
  //
  // If 'throughCppComments', back up through any C++ comments as well
  // (but not C comments).
  void backupToWSStart(bool throughCppComments);

  // Move backward until we are just past a newline.
  void backupToLineStart();

  // Move the cursor backwards by 'amt', silently stopping if we hit the
  // lower bound.
  void backup(unsigned amt);

  // Move forward until we are just past the last #include such that
  // all of the text between the start and end locations is #includes,
  // whitespace, and comments.
  void advancePastContiguousIncludes();

  // Skip past whitespace, returning true if any was skipped.
  bool skipWS();

  // Skip past non-whitespace, return true if any skipped.
  bool skipNonWS();

  // If the cursor is on the start of a C comment, skip to the first
  // character after that comment and return true.  Otherwise return
  // false.  If the comment is unterminated, return true and skip to the
  // upper bound.
  bool skipCCommentIf();

  // If the cursor is on the start of a C++ comment, skip to the first
  // character after it (after the newline) and return true.  Otherwise
  // return false.
  bool skipCppCommentIf();

  // Skip all comments and whitespace, returning true if any was
  // skipped.
  bool skipCommentsAndWhitespace();

  // If the cursor is on "#include", skip past that line.
  bool skipIncludeIf();

  // If looking at 'prefix', then skip it and return true; else false.
  bool skipStringIf(char const *prefix);

  // Return true if the characters at the cursor match 'prefix'.
  bool lookingAt(char const *prefix) const;

  // Search ahead for 'searchString'.  If it is found, leave the cursor
  // pointing at the first character *after* that string, and return
  // true.  Otherwise return false with an unchanged cursor.
  bool searchFor(char const *searchString);

  // Search forward for the next whitespace-separated token, and return
  // its text, leaving the cursor on the character just after the last
  // one in the returned text.  Return "" if there are no more
  // non-whitespace characters before the upper bound.
  std::string getNextWSSeparatedToken();

  // Collect characters satisfying the constraints of a C identifier,
  // starting at the cursor.  Stop when the first non-identifier
  // character is encountered, leaving the cursor there and not
  // including it in the returned value.  This returns an empty string
  // (without advancing!) if the first character does not conform.
  std::string getNextIdentifier();

  // Return all text from the cursor up to, but not including, the
  // character at 'endOffset', or at the upper bound, whichever is less.
  // If 'endOffset' is at or less than the cursor, return the empty
  // string.
  std::string textUpTo(unsigned endOffset);

  // Return the 1-based line/col of the current cursor position, taking
  // the lower bound position as the (1,1) start position.  This does a
  // linear scan, so is not very efficient.
  void getLineCol(int /*OUT*/ &line, int /*OUT*/ &col) const;

  // Return line/col as "<line>:<col>".
  std::string getLineColStr() const;

  // Fail an assertion if invariants do not hold.
  void assertInvariants() const;
};


// Unit tests, defined in stringref-parse-test.cc.
void stringref_parse_unit_tests();


#endif // STRINGREF_PARSE_H
