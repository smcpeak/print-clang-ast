// map-util.h
// Utilities for std::map.

#ifndef MAP_UTIL_H
#define MAP_UTIL_H

#include <map>                         // std::map
#include <optional>                    // std::optional
#include <utility>                     // std::make_pair

#include <assert.h>                    // assert


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


#endif // MAP_UTIL_H
