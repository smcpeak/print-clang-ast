// compare-util.h
// Utilities for implementing three-way compare.

#ifndef COMPARE_UTIL_H
#define COMPARE_UTIL_H

#include <functional>                  // std::less


// Return -1 if a<b, +1 if a>b, and 0 otherwise.
template <class NUM>
int compare(NUM const &a, NUM const &b)
{
  // Use this instead of '<' so this works with pointers without relying
  // on implementation-defined behavior.
  std::less<NUM> lt;

  if (lt(a, b)) {
    return -1;
  }
  else if (lt(b, a)) {
    return +1;
  }
  else {
    return 0;
  }
}


// Compare member 'memb' from objects 'a' and 'b', returning the
// comparison result if they are not equal.  This is meant to be used as
// part of a comparison chain.
#define COMPARE_MEMB(memb)                 \
  if (int ret = compare(a.memb, b.memb)) { \
    return ret;                            \
  }


// Define a single friend comparison operator.
#define DEFINE_ONE_FRIEND_OPERATOR(Class, op)              \
  friend bool operator op (Class const &a, Class const &b) \
    { return compare(a,b) op 0; }


// Declare a set of friend comparison operators, assuming that a
// 'compare' function exists.
#define DEFINE_FRIEND_OPERATORS(Class)  \
  DEFINE_ONE_FRIEND_OPERATOR(Class, ==) \
  DEFINE_ONE_FRIEND_OPERATOR(Class, !=) \
  DEFINE_ONE_FRIEND_OPERATOR(Class, < ) \
  DEFINE_ONE_FRIEND_OPERATOR(Class, <=) \
  DEFINE_ONE_FRIEND_OPERATOR(Class, > ) \
  DEFINE_ONE_FRIEND_OPERATOR(Class, >=)


#endif // COMPARE_UTIL_H
