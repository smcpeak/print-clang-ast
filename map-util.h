// map-util.h
// Utilities for std::map.

#ifndef MAP_UTIL_H
#define MAP_UTIL_H

#include <cassert>                     // assert
#include <iostream>                    // std::ostream
#include <map>                         // std::map
#include <optional>                    // std::optional
#include <utility>                     // std::make_pair


// I don't like to have to say 'make_pair' all the time, and I'm
// ambivalent about using the initializer list syntax, so this function
// encapsulates insertion.
//
// Returns true if the item was inserted (otherwise, it was already in
// the set).
template <class K, class V>
bool mapInsert(std::map<K,V> &m, K const &k, V const &v)
{
  auto res = m.insert(std::make_pair(k, v));
  return res.second;
}


template <class K, class V>
void mapInsertUnique(std::map<K,V> &m, K const &k, V const &v)
{
  bool res = mapInsert(m, k, v);
  assert(res);
}


template <class K, class V>
void mapInsertAll(std::map<K,V> &dest, std::map<K,V> const &src)
{
  for (auto const &kv : src) {
    dest.insert(kv);
  }
}


// Insert all keys from 'src', presumably a map, into 'dest', presumably
// a set.
template <class DestSet, class SrcMap>
void mapInsertAllKeys(DestSet &dest, SrcMap const &src)
{
  for (auto const &kv : src) {
    dest.insert(kv.first);
  }
}


// Compute and return a map from value to key.  This asserts that the
// values are unique.
template <class K, class V>
std::map<V,K> mapInvert(std::map<K,V> const &src)
{
  std::map<V,K> dest;
  for (auto const &kv : src) {
    mapInsertUnique(dest, kv.second, kv.first);
  }
  return dest;
}


// Return an optional iterator from a map lookup.
template <class K, class V>
std::optional<typename std::map<K,V>::const_iterator> mapFindOpt(
  std::map<K,V> const &m,
  K const &k)
{
  auto it = m.find(k);
  if (it != m.end()) {
    return std::make_optional(it);
  }
  else {
    return std::optional<typename std::map<K,V>::const_iterator>();
  }
}


template <class K, class V, class PRINT_KEY, class PRINT_VALUE>
void mapWrite(
  std::ostream &os,
  std::map<K,V> const &m,
  PRINT_KEY printKey,
  PRINT_VALUE printValue)
{
  os << "{";

  int ct = 0;
  for (auto const &kv : m) {
    if (ct > 0) {
      os << ",";
    }
    os << " ";
    printKey(os, kv.first);
    os << ": ";
    printValue(os, kv.second);
    ++ct;
  }

  if (ct > 0) {
    os << " ";
  }
  os << "}";
}


// Like above, but writing an array of pairs instead of a JSON map,
// since JSON maps can only have strings as keys.
template <class K, class V, class PRINT_KEY, class PRINT_VALUE>
void mapWriteAsArray(
  std::ostream &os,
  std::map<K,V> const &m,
  PRINT_KEY printKey,
  PRINT_VALUE printValue)
{
  os << "[";

  int ct = 0;
  for (auto const &kv : m) {
    if (ct > 0) {
      os << ",";
    }
    os << " [";
    printKey(os, kv.first);
    os << ", ";
    printValue(os, kv.second);
    os << "]";
    ++ct;
  }

  if (ct > 0) {
    os << " ";
  }
  os << "]";
}


// This has to be put into 'std', otherwise it is not found by ADL in
// certain situations, such as when using my 'stringb' macro.
namespace std {
  template <class K, class V>
  std::ostream& operator<< (std::ostream &os, std::map<K,V> const &m)
  {
    mapWrite(
      os,
      m,
      [](std::ostream &os, K const &k) -> void {
        os << k;
      },
      [](std::ostream &os, V const &v) -> void {
        os << v;
      });

    return os;
  }
}


#endif // MAP_UTIL_H
