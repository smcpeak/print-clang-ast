// util-macros.h
// Various utility macros.

#ifndef UTIL_MACROS_H
#define UTIL_MACROS_H

// This file is intended to be very lightweight in terms of
// dependencies.


// Pseudo-attribute meaning a pointer can be nullptr.
#define NULLABLE /*nullable*/


// Place in a class definition to inhibit the auto-generated copy
// operations.
#define NO_OBJECT_COPIES(name)              \
  name(name&) = delete;                     \
  void operator=(name&) = delete /*user ;*/


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


#endif // UTIL_MACROS_H
