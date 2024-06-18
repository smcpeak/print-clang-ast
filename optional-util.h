// optional-util.h
// Utilities related to std::optional.

#ifndef PCA_OPTIONAL_UTIL_H
#define PCA_OPTIONAL_UTIL_H

#include <iostream>                    // std::ostream
#include <optional>                    // std::optional
#include <string>                      // std::string
#include <sstream>                     // std::ostringstream


// Convert 'o' to a string using its insert operator, or 'ifNone' if 'o'
// does not contain a value.
template <class T>
std::string optionalToString(std::optional<T> const &o, char const *ifNone)
{
  if (o.has_value()) {
    std::ostringstream oss;
    oss << o.value();
    return oss.str();
  }
  else {
    return ifNone;
  }
}


template <class T>
std::ostream& operator<< (std::ostream &os, std::optional<T> const &opt)
{
  if (opt.has_value()) {
    os << opt.value();
  }
  else {
    // This assumes 'null' will not be confused with whatever 'T' is.
    // That's not true in every possible case, but in practice it almost
    // always is, and I can handle exceptions separately.
    os << "null";
  }
  return os;
}


#endif // PCA_OPTIONAL_UTIL_H
