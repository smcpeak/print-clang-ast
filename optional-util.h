// optional-util.h
// Utilities related to std::optional.

#ifndef OPTIONAL_UTIL_H
#define OPTIONAL_UTIL_H

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


#endif // OPTIONAL_UTIL_H
