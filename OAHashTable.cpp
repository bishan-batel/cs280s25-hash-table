#pragma once

#include "OAHashTable.h"
#include <algorithm>
#include <cstring>
#include <utility>

template<typename T>
OAHashTable<T>::OAHashTable(const OAHTConfig& config):
    config{config}, capacity{config.InitialTableSize_} {

  stats.PrimaryHashFunc_ = config.PrimaryHashFunc_;
  stats.SecondaryHashFunc_ = config.SecondaryHashFunc_;

  // initialise table
  slots = new Slot[capacity]{};
}

template<typename T>
OAHashTable<T>::OAHashTable(OAHashTable&& from):
    config{from.config},
    stats{from.stats},
    slots{std::exchange(from.slots, nullptr)},
    capacity{std::exchange(from.capacity, 0)} {}

template<typename T>
OAHashTable<T>::OAHashTable(const OAHashTable& from):
    config{from.config}, stats{from.stats}, capacity{from.capacity} {

  slots = new Slot[capacity];
  std::copy(from.slots, &from.slots[capacity], slots);
}

template<typename T>
auto OAHashTable<T>::operator=(OAHashTable&& from) -> OAHashTable& {
  config = from.config;
  stats = from.stats;
  slots = std::exchange(from.slots, nullptr);
  capacity = std::exchange(from.capacity, 0);
  return *this;
}

template<typename T>
auto OAHashTable<T>::operator=(const OAHashTable& from) -> OAHashTable& {
  if (&from == this) {
    return *this;
  }
  destruct();

  config = from.config;
  stats = from.stats;
  capacity = from.capacity;

  slots = new Slot[capacity];
  std::copy(from.slots, &from.slots[capacity], slots);

  return *this;
}

template<typename T>
OAHashTable<T>::~OAHashTable() {
  destruct();
}

template<typename T>
auto OAHashTable<T>::insert(const char* key, const T& data) -> void {}

template<typename T>
auto OAHashTable<T>::remove(const char* key) -> void {}

template<typename T>
auto OAHashTable<T>::find(const char* key) const -> const T& {
  stats.Probes_++;
}

template<typename T>
auto OAHashTable<T>::clear() -> void {

  for (usize i = 0; i < capacity and size > 0; i++) {
    Slot& slot = slots[i];

    if (slot.State != Slot::UNOCCUPIED) {
      continue;
    }
    size--;
  }

  // if () {
  // config.FreeProc_();
  // }
}

template<typename T>
auto OAHashTable<T>::GetStats() const -> OAHTStats {
  return stats;
}

template<typename T>
auto OAHashTable<T>::GetTable() const -> const OAHTSlot* {
  return slots.get();
}

template<typename T>
auto OAHashTable<T>::init_table() -> void {}

template<typename T>
auto OAHashTable<T>::grow() -> void {
  stats.Expansions_++;

  usize new_capacity{static_cast<usize>(config.GrowthFactor_ * capacity)};
  new_capacity = GetClosestPrime(new_capacity);

  Slot* new_slots = new Slot[new_capacity]{};

  for (usize i = 0; i < capacity; i++) {
    new_slots[i] = slots[i];
  }

  capacity = new_capacity;
  slots = new_slots;
}

template<typename T>
auto OAHashTable<T>::index_of(const char* key) const -> index_res {
  const usize hash1 = primary_hash(key);
  const usize hash2 = secondary_hash(key);

  for (usize i = 0; i < capacity; i++) {
    const usize index{(hash1 + i) % capacity};
    Slot& slot{slots[index]};

    if (slot.key_matches(key)) {
      return {slot, index};
    }
  }

  return {nullptr, 0};
}

template<typename T>
auto OAHashTable<T>::destruct() -> void {
  clear();
}

template<typename T>
auto OAHashTable<T>::primary_hash(const char* key) -> usize {
  config.PrimaryHashFunc_(key);
}

template<typename T>
auto OAHashTable<T>::Slot::key_matches(const char* key) -> bool {
  return std::strncmp(Key, key, MAX_KEYLEN);
}

template<typename T>
auto OAHashTable<T>::secondary_hash(const char* key) -> usize {
  if (config.SecondaryHashFunc_ == nullptr) {
    return 0;
  }

  return config.SecondaryHashFunc_(key);
}
