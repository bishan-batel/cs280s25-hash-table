#pragma once

#include "OAHashTable.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>

// ============================================================================
// Lifetime / Rule of 5 Semantics
// ============================================================================

template<typename T>
OAHashTable<T>::OAHashTable(const OAHTConfig& config): config{config} {

  stats.PrimaryHashFunc_ = config.PrimaryHashFunc_;
  stats.SecondaryHashFunc_ = config.SecondaryHashFunc_;

  capacity() = config.InitialTableSize_;

  // initialise table
  slots.reset(new Slot[capacity()]{});
}

template<typename T>
OAHashTable<T>::OAHashTable(OAHashTable&& from):
    config{from.config},
    stats{from.stats},
    slots{std::exchange(from.slots, nullptr)} {}

template<typename T>
OAHashTable<T>::OAHashTable(const OAHashTable& from):
    config{from.config}, stats{from.stats} {

  slots.reset(new Slot[from.capacity()]);
  std::copy(from.slots, &from.slots[from.capacity()], slots);
}

template<typename T>
auto OAHashTable<T>::operator=(OAHashTable&& from) -> OAHashTable& {
  config = from.config;
  stats = from.stats;
  slots.reset(std::exchange(from.slots, nullptr));
  return *this;
}

template<typename T>
auto OAHashTable<T>::operator=(const OAHashTable& from) -> OAHashTable& {
  if (&from == this) {
    return *this;
  }

  clear();

  config = from.config;
  stats = from.stats;

  slots.reset(new Slot[capacity()]);
  std::copy(from.slots, &from.slots[from.capacity()], slots);

  return *this;
}

template<typename T>
OAHashTable<T>::~OAHashTable() {
  clear();
}

// ============================================================================
// Public API
// ============================================================================

template<typename T>
auto OAHashTable<T>::insert(const char* key, const T& data) -> void {
  Slot* slot = index_of(key).slot;

  if (slot) {
    slot->Data = data;
    return;
  }

  size()++;
  grow_if_needed();

  const usize hash1 = hash(key);
  const usize stride = probe_stride(key);

  for (usize i = 0; i < capacity(); i += stride) {
    const usize index{(hash1 + i) % capacity()};
    Slot& slot{slots[index]};

    stats.Probes_++;
    if (slot.State != OAHashTable::Slot::OCCUPIED) {
      slot.State = OAHashTable::Slot::OCCUPIED;
      std::strncpy(slot.Key, key, MAX_KEYLEN - 1);
      slot.Data = data;
      return;
    }
  }

  // Unreachable;
  throw OAHashTableException(OAHashTableException::E_NO_MEMORY, "TODO Insert");
}

template<typename T>
auto OAHashTable<T>::remove(const char* key) -> void {
  (void)key;
  throw OAHashTableException(OAHashTableException::E_NO_MEMORY, "TODO Remove");
}

template<typename T>
auto OAHashTable<T>::find(const char* key) const -> const T& {
  const Slot* slot = index_of(key).slot;

  if (slot) {
    return slot->Data;
  }

  throw OAHashTableException(
    OAHashTableException::E_ITEM_NOT_FOUND,
    "Item not found"
  );
}

template<typename T>
auto OAHashTable<T>::clear() -> void {

  for (usize i = 0; i < capacity() and size() > 0; i++) {
    Slot& slot = slots[i];

    stats.Probes_++;
    if (slot.State != Slot::OCCUPIED) {
      continue;
    }
    size()--;

    if (config.FreeProc_) {
      config.FreeProc_(std::move(slot.Data));
    }
  }
  assert(empty());
}

// ============================================================================
// Internal Buffer Manaagement
// ============================================================================

template<typename T>
auto OAHashTable<T>::grow_if_needed() -> void {
  if (size() > capacity() or load_factor() > config.MaxLoadFactor_) {
    grow();
  }
}

template<typename T>
auto OAHashTable<T>::grow() -> void {
  stats.Expansions_++;

  const u32 new_capacity{
    GetClosestPrime(static_cast<u32>(config.GrowthFactor_ * capacity())),
  };

  try {
    std::unique_ptr<Slot[]> new_slots{new Slot[new_capacity]{}};

    for (usize i = 0; i < capacity(); i++) {
      new_slots[i] = slots[i];
    }

    capacity() = new_capacity;
    slots = std::move(new_slots);
  } catch (const std::bad_alloc&) {
    throw OAHashTableException(
      OAHashTableException::E_NO_MEMORY,
      "std::bad_alloc thrown: no memory"
    );
  }
}

template<typename T>
auto OAHashTable<T>::index_of(const char* key) const -> index_res {
  const usize hash1 = hash(key);
  const usize stride = probe_stride(key);

  const usize capacity = this->capacity();

  for (usize i = 0; i < capacity; i += stride) {
    const usize index{(hash1 + i) % capacity};
    Slot& slot{slots[index]};

    slot.probes++;
    stats.Probes_++;

    if (slot.State != OAHashTable::Slot::OCCUPIED) {
      continue;
    }

    if (slot.key_matches(key)) {
      return {&slot, index};
    }
  }

  return {nullptr, 0};
}

template<typename T>
auto OAHashTable<T>::hash(const char* key) const -> usize {
  return config.PrimaryHashFunc_(key, capacity());
}

template<typename T>
auto OAHashTable<T>::Slot::key_matches(const char* key) const -> bool {
  return std::strncmp(Key, key, MAX_KEYLEN);
}

template<typename T>
auto OAHashTable<T>::probe_stride(const char* key) const -> usize {
  if (config.SecondaryHashFunc_ == nullptr) {
    return 1;
  }

  return config.SecondaryHashFunc_(key, capacity() - 1) + 1;
}

// ============================================================================
// Getters
// ============================================================================

template<typename T>
auto OAHashTable<T>::size() const -> u32 {
  return stats.Count_;
}

template<typename T>
auto OAHashTable<T>::capacity() const -> u32 {
  return stats.TableSize_;
}

template<typename T>
auto OAHashTable<T>::size() -> u32& {
  return stats.Count_;
}

template<typename T>
auto OAHashTable<T>::capacity() -> u32& {
  return stats.TableSize_;
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
auto OAHashTable<T>::load_factor() const -> f32 {
  return static_cast<f32>(size()) / static_cast<f32>(capacity());
}

template<typename T>
auto OAHashTable<T>::empty() const -> bool {
  return size() == 0;
}
