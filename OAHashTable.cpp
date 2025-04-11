#pragma once

#include "OAHashTable.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

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
    config{std::exchange(from.config, {})},
    stats{std::exchange(from.stats, {})},
    slots{std::exchange(from.slots, nullptr)} {}

template<typename T>
OAHashTable<T>::OAHashTable(const OAHashTable& from):
    config{from.config}, stats{from.stats} {

  slots.reset(new Slot[from.capacity()]);
  std::vector<T> a;
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
  grow_if_needed();
  size()++;

  const usize hash1 = hash(key);
  const usize stride = probe_stride(key);

  for (usize i = 0; i < capacity(); i++) {
    const usize index{(hash1 + i * stride) % capacity()};
    Slot& slot{slots[index]};

    stats.Probes_++;
    if (slot.State == Slot::OCCUPIED) {
      continue;
    }

    if (slot.State == Slot::UNOCCUPIED) {
      slot.State = OAHashTable::Slot::OCCUPIED;
      std::strncpy(slot.Key, key, MAX_KEYLEN - 1);
      slot.Data = data;
      return;
    }

    for (usize j = 1; j < capacity(); j++) {
      stats.Probes_++;
      if (slots[(hash1 + j * stride) % capacity()].State != Slot::DELETED) {
        continue;
      }
      if (slots[(hash1 + j * stride) % capacity()].key_matches(key)) {
        throw OAHashTableException(
          OAHashTableException::E_DUPLICATE,
          "Duplicate key"
        );
      } else {
        break;
      }
    }

    slot.State = OAHashTable::Slot::OCCUPIED;
    std::strncpy(slot.Key, key, MAX_KEYLEN - 1);
    slot.Data = data;
    return;
  }
}

template<typename T>
auto OAHashTable<T>::remove(const char* key) -> void {
  const u32 hash1 = hash(key);
  const u32 stride = probe_stride(key);

  for (u32 i = 0; i < capacity(); i++) {
    const u32 index{(hash1 + i * stride) % capacity()};
    Slot& slot{slots[index]};

    stats.Probes_++;
    if (slot.State == Slot::UNOCCUPIED) {
      throw OAHashTableException(
        OAHashTableException::E_ITEM_NOT_FOUND,
        "Key not in table."
      );
      return;
    }

    if (not slot.key_matches(key)) {
      continue;
    }

    if (slot.State == Slot::DELETED) {
      throw OAHashTableException(
        OAHashTableException::E_ITEM_NOT_FOUND,
        "Key not in table."
      );
    }

    if (config.FreeProc_) {
      config.FreeProc_(slot.Data);
    }

    size()--;
    if (config.DeletionPolicy_ == OAHTDeletionPolicy::MARK) {
      slot.State = Slot::DELETED;
    } else if (config.DeletionPolicy_ == OAHTDeletionPolicy::PACK) {
      slot.State = Slot::UNOCCUPIED;

      for (u32 j = 1; j < capacity(); j++) {
        const u32 k = (index + j) % capacity();

        if (slots[k].State != Slot::OCCUPIED) {
          break;
        }

        slots[k].State = Slot::UNOCCUPIED;
        size()--;
        insert(slots[k].Key, slots[k].Data);
      }
    }
    return;
  }

  throw OAHashTableException(
    OAHashTableException::E_ITEM_NOT_FOUND,
    "Key not in table."
  );
}

template<typename T>
auto OAHashTable<T>::find(const char* key) const -> const T& {
  const Slot* slot = index_of(key).slot;

  if (slot) {
    return slot->Data;
  }

  throw OAHashTableException(
    OAHashTableException::E_ITEM_NOT_FOUND,
    "Item not found in table."
  );
}

template<typename T>
auto OAHashTable<T>::clear() -> void {

  for (usize i = 0; i < capacity() and size() > 0; i++) {
    Slot& slot = slots[i];

    if (std::exchange(slot.State, Slot::UNOCCUPIED) != Slot::OCCUPIED) {
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
  const f32 load_factor{
    static_cast<f32>(size() + 1) / static_cast<f32>(capacity())
  };

  if (size() + 1 > capacity() or load_factor > config.MaxLoadFactor_) {
    grow();
  }
}

template<typename T>
auto OAHashTable<T>::grow() -> void {
  stats.Expansions_++;

  const u32 old_capacity = capacity();

  capacity() = GetClosestPrime( //
    static_cast<u32>(std::ceil(config.GrowthFactor_ * old_capacity))
  );

  const u32 old_size = std::exchange(size(), 0);

  try {
    std::unique_ptr<Slot[]> old_slots{new Slot[capacity()]{}};
    slots.swap(old_slots);

    for (u32 i = 0; i < old_capacity and size() < old_size; i++) {
      Slot& slot = old_slots[i];

      if (slot.State != slot.OCCUPIED) {
        continue;
      }

      insert(slot.Key, slot.Data);
    }

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

    stats.Probes_++;

    if (slot.State == Slot::UNOCCUPIED) {
      return {nullptr, 0};
    }

    if (slot.State == Slot::DELETED) {
      if (slot.key_matches(key)) {
        return {nullptr, 0};
      }
    }

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
auto OAHashTable<T>::hash(const char* key) const -> u32 {
  return config.PrimaryHashFunc_(key, capacity());
}

template<typename T>
auto OAHashTable<T>::Slot::key_matches(const char* key) const -> bool {
  return std::strncmp(Key, key, MAX_KEYLEN) == 0;
}

template<typename T>
auto OAHashTable<T>::probe_stride(const char* key) const -> u32 {
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
