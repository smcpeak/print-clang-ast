// enum-util.h
// Utilities for working with enumerations and bit sets.

#ifndef ENUM_UTIL_H
#define ENUM_UTIL_H

#include "sm-pp-util.h"                // SM_PP_MAP_WITH_ARG

#include <string>                      // std::string


#define ENUM_TABLE_LOOKUP_ENTRY(scopeQualifier, enumerator) \
  { scopeQualifier enumerator, #enumerator },


// For some enumerated type, and a set of enumerators, if 'key' matches
// one of them, return the associated name.
//
// This uses simple linear search for simplicity, and because the
// enumerations I use it with are not always densely populated.
#define ENUM_TABLE_LOOKUP(scopeQualifier, EnumType, key, enumerators, ...) \
  static struct Entry {                                                    \
    scopeQualifier EnumType m_key;                                         \
    char const *m_value;                                                   \
  } const entries[] = {                                                    \
    SM_PP_MAP_WITH_ARG(ENUM_TABLE_LOOKUP_ENTRY, scopeQualifier,            \
                       enumerators, __VA_ARGS__)                           \
  };                                                                       \
                                                                           \
  for (Entry const &e : entries) {                                         \
    if (e.m_key == key) {                                                  \
      return e.m_value;                                                    \
    }                                                                      \
  }


// Like above, but if the lookup fails, return a string, constructed
// with 'stringb', that looks like a construction-style cast of 'key' to
// 'EnumType'
#define ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(scopeQualifier, EnumType, key, enumerators, ...) \
  ENUM_TABLE_LOOKUP(scopeQualifier, EnumType, key, enumerators, __VA_ARGS__)               \
                                                                                           \
  /* Key is not found. */                                                                  \
  return stringb(#EnumType "(" << (int)key << ")");


// Like 'ENUM_TABLE_LOOKUP', but for an enum class, such that we need
// to qualify the enumerators with 'EnumType'.
#define ENUM_CLASS_TABLE_LOOKUP(scopeQualifier, EnumType, key, enumerators, ...) \
  static struct Entry {                                                          \
    scopeQualifier EnumType m_key;                                               \
    char const *m_value;                                                         \
  } const entries[] = {                                                          \
    SM_PP_MAP_WITH_ARG(ENUM_TABLE_LOOKUP_ENTRY,                                  \
                       scopeQualifier EnumType::,                                \
                       enumerators, __VA_ARGS__)                                 \
  };                                                                             \
                                                                                 \
  for (Entry const &e : entries) {                                               \
    if (e.m_key == key) {                                                        \
      return e.m_value;                                                          \
    }                                                                            \
  }


// Then the corresponding _OR_STRINGB variant.
#define ENUM_CLASS_TABLE_LOOKUP_OR_STRINGB_CAST(scopeQualifier, EnumType, key, enumerators, ...) \
  ENUM_CLASS_TABLE_LOOKUP(scopeQualifier, EnumType, key, enumerators, __VA_ARGS__)               \
                                                                                                 \
  /* Key is not found. */                                                                        \
  return stringb(#EnumType "(" << static_cast<int>(key) << ")");


// Represent a single flag value in a bit set.
struct BitflagsEntry {
  unsigned m_flag;
  char const *m_name;
};


// Stringify 'flags' as a '|'-separated sequence of flag names.
std::string bitflagsString(
  BitflagsEntry const *beginEntries,
  BitflagsEntry const *endEntries,
  unsigned flags,
  char const *typeName,
  char const *zeroName);


// Emit code to stringify 'flags'.
#define BITFLAGS_TABLE_LOOKUP(scopeQualifier, typeName, zeroName, flags, enumerators, ...) \
  static BitflagsEntry const entries[] = {                                               \
    SM_PP_MAP_WITH_ARG(ENUM_TABLE_LOOKUP_ENTRY, scopeQualifier,                          \
                       enumerators, __VA_ARGS__)                                         \
  };                                                                                     \
                                                                                         \
  return bitflagsString(                                                                 \
    entries, *(&entries + 1),                                                            \
    flags,                                                                                 \
    typeName,                                                                            \
    zeroName);


#endif // ENUM_UTIL_H
