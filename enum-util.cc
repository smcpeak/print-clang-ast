// enum-util.cc
// Code for enum-util.h.

#include "enum-util.h"                           // this module

#include <sstream>                               // std::ostringstream


// Stringify 'flags' as a '|'-separated sequence of flag names.
std::string bitflagsString(
  BitflagsEntry const *beginEntries,
  BitflagsEntry const *endEntries,
  unsigned flags,
  char const *typeName,
  char const *zeroName)
{
  std::ostringstream oss;

  int ct=0;
  for (BitflagsEntry const *e = beginEntries; e < endEntries; ++e) {
    if (e->m_flag & flags) {
      if (ct++ > 0) {
        oss << '|';
      }
      oss << e->m_name;
      flags &= ~e->m_flag;
    }
  }

  if (flags) {
    if (ct++ > 0) {
      oss << '|';
    }
    oss << typeName << "(0x" << std::hex << flags << ")";
  }

  if (ct == 0) {
    oss << zeroName;
  }

  return oss.str();
}


// EOF
