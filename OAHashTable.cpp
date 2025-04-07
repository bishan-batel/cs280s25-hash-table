#pragma once

#include "OAHashTable.h"

template<typename T>
OAHashTable<T>::OAHashTable(const OAHTConfig& Config) {}

template<typename T>
OAHashTable<T>::OAHashTable(OAHashTable&& from) {}

template<typename T>
OAHashTable<T>::OAHashTable(const OAHashTable& from) {}

template<typename T>
auto OAHashTable<T>::operator=(OAHashTable&& from) -> OAHashTable& {}

template<typename T>
auto OAHashTable<T>::operator=(const OAHashTable& from) -> OAHashTable& {}

template<typename T>
OAHashTable<T>::~OAHashTable() {}

template<typename T>
auto OAHashTable<T>::insert(const char* key, const T& data) -> void {}

template<typename T>
auto OAHashTable<T>::remove(const char* key) -> void {}

template<typename T>
auto OAHashTable<T>::find(const char* key) const -> const T& {
  stats.Probes_++;
}

template<typename T>
auto OAHashTable<T>::clear() -> void {}

template<typename T>
auto OAHashTable<T>::GetStats() const -> OAHTStats {
  return stats;
}

template<typename T>
auto OAHashTable<T>::GetTable() const -> const OAHTSlot* {
  return &stats;
}

template<typename T>
auto OAHashTable<T>::init_table() -> void {}

template<typename T>
auto OAHashTable<T>::grow() -> void {
  stats.Expansions_++;
}

template<typename T>
auto OAHashTable<T>::index_of(const char* Key, OAHTSlot*& slot) const -> i32 {}
