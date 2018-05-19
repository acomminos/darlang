#ifndef DARLANG_SRC_UTIL_SCOPED_MAP_H_
#define DARLANG_SRC_UTIL_SCOPED_MAP_H_

namespace darlang {
namespace util {

// A scopable map, intended to be used to implement function and block
// subscoping of identifier mappings.
// Lookups are linear in the number of parent scopes (in the worst-case).
// TODO(acomminos): implement caching for parent retrievals?
template <typename K, typename V, V DefaultValue = V()>
class ScopedMap {
 public:
  // Creates a new unparented, top-level ScopedMap.
  ScopedMap() : parent_(nullptr) {}
  // Creates a new ScopedMap falling back to the given parent for unknown
  // identifiers.
  ScopedMap(ScopedMap const* parent) : parent_(parent) {}

  // Attempts to find the value associated with the given key in the ScopedMap.
  // If this ScopedMap does not contain a definition for the value, recurses on
  // the parent ScopedMap.
  // If no definition could be found, returns DefaultValue.
  V Lookup(K key) const {
    auto it = map_.find(key);
    if (it != map_.end()) {
      return it->second;
    }
    if (parent_ != nullptr) {
      return parent_->Lookup(key);
    }
    return DefaultValue;
  }

  // Assigns a key to a value in this ScopedMap.
  void Assign(K key, V value) {
    map_[key] = value;
  }

 private:
  const ScopedMap* const parent_;
  std::unordered_map<K, V> map_;
};

}  // namespace util
}  // namespace darlang

#endif  // DARLANG_SRC_UTIL_SCOPED_MAP_H_
