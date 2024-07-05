// macro-expansion.cc
// A dependency arising from a macro expansion is hidden.

#include "macro-definition.h"          // visibleMacro

int f(int x)
{
  visibleMacro();                      // SYMLINE(expansionLine)

  return x;                            // SYMLINE(returnLine)
}

// EOF
