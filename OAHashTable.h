#pragma once

#include <cstdlib>
#ifndef OAHASHTABLEH
#define OAHASHTABLEH

#include "Support.h"
#include <cstdint>
#include <string>
#include <memory>

/**
 * @brief 32 Bit Floating Point Number
 */
using f32 = float;

/**
 * @brief 64 Bit Floating Point Number
 */
using f64 = double;

/**
 * @brief Fix Sized Unsigned 8 Bit Integer (cannot be negative)
 */
using u8 = std::uint8_t;

/**
 * @brief Fix Sized Unsigned 16 Bit Integer (cannot be negative)
 */
using u16 = std::uint16_t;

/**
 * @brief Fix Sized Unsigned 32 Bit Integer (cannot be negative)
 */
using u32 = std::uint32_t;

/**
 * @brief Fix Sized Unsigned 64 Bit Integer (cannot be negative)
 */
using u64 = unsigned long int;

/**
 * @brief Biggest Unsigned Integer type that the current platform can use
 * (cannot be negative)
 */
using umax = std::uintmax_t;

/**
 * @brief Unsigned Integer for when referring to any form of memory size or
 * offset (eg. an array length or index)
 */
using usize = std::size_t;
/**
 * @brief Unsigned Integer Pointer typically used for pointer arithmetic
 */
using uptr = std::uintptr_t;

/**
 * @brief Signed 8 bit Integer
 */
using i8 = std::int8_t;

/**
 * @brief Signed 16 bit Integer
 */
using i16 = std::int16_t;

/**
 * @brief Signed 32 bit Integer
 */
using i32 = std::int32_t;

/**
 * @brief Signed 64 bit Integer
 */
using i64 = std::int64_t;

/**
 * @brief Integer pointer typically used for pointer arithmetic
 */
using iptr = std::intptr_t;

/*!
client-provided hash function: takes a key and table size,
returns an index in the table.
*/
using HASHFUNC = u32 (*)(const char*, u32);

//! Max length of our "string" keys
const usize MAX_KEYLEN = 32;

//! The exception class for the hash table
class OAHashTableException {

public:

  //! Possible exception conditions
  enum Code {
    E_ITEM_NOT_FOUND,
    E_DUPLICATE,
    E_NO_MEMORY
  };

  using OAHASHTABLE_EXCEPTION = Code;

  //! Non-default constructor
  inline OAHashTableException(Code err, std::string message):
      error{err}, message{std::move(message)} {};

  /*!
    Retrieves exception code

    @return
      One of: E_ITEM_NOT_FOUND, E_DUPLICATE, E_NO_MEMORY
  */
  inline virtual Code code() const { return error; }

  /**
   * Retrieve human-readable string describing the exception
   *
   * @return The description of the exception
   */
  inline virtual const char* what() const { return message.c_str(); }

private:

  Code error;          //!< Code for the exception
  std::string message; //!< Readable string describing the exception
};

//! The policy used during a deletion
enum OAHTDeletionPolicy {
  MARK,
  PACK
};

//! OAHashTable statistical info
struct OAHTStats {
  //! Default constructor
  OAHTStats() = default;

  u32 Count_{0};                        //!< Number of elements in the table
  u32 TableSize_{0};                    //!< Size of the table (total slots)
  u32 Probes_{0};                       //!< Number of probes performed
  u32 Expansions_{0};                   //!< Number of times the table grew
  HASHFUNC PrimaryHashFunc_{nullptr};   //!< Pointer to primary hash function
  HASHFUNC SecondaryHashFunc_{nullptr}; //!< Pointer to secondary hash function
};

//! Hash table definition (open-addressing)
template<typename T>
class OAHashTable {
public:

  /**
   * Client-provided free proc (we own the data)
   */
  using FREEPROC = void (*)(T);

  //! Configuration for the hash table
  struct OAHTConfig {
    //! Non-default constructor
    inline OAHTConfig(
      u32 initial_size,
      HASHFUNC primary_hash,
      HASHFUNC second_hash = nullptr,
      f64 max_load_factor = 0.5,
      f64 grow_factor = 2.0,
      OAHTDeletionPolicy policy = PACK,
      FREEPROC free_proc = nullptr
    ):
        InitialTableSize_{initial_size},
        PrimaryHashFunc_{primary_hash},
        SecondaryHashFunc_{second_hash},
        MaxLoadFactor_{max_load_factor},
        GrowthFactor_{grow_factor},
        DeletionPolicy_{policy},
        FreeProc_{free_proc} {}

    u32 InitialTableSize_;              //!< The starting table size
    HASHFUNC PrimaryHashFunc_;          //!< First hash function
    HASHFUNC SecondaryHashFunc_;        //!< Hash function to resolve collisions
    f64 MaxLoadFactor_;                 //!< Maximum LF before growing
    f64 GrowthFactor_;                  //!< The amount to grow the table
    OAHTDeletionPolicy DeletionPolicy_; //!< MARK or PACK
    FREEPROC FreeProc_;                 //!< Client-provided free function
  };

  //! The 3 possible states the slot can be in

  //! Slots that will hold the key/data pairs
  struct Slot {
    enum SlotState {
      OCCUPIED,
      UNOCCUPIED,
      DELETED
    };

    using OAHTSlot_State = SlotState;

    char Key[MAX_KEYLEN]{""};    //!< Key is a string
    T Data;                      //!< Client data
    SlotState State{UNOCCUPIED}; //!< The state of the slot
    i32 probes{0};               //!< For testing

    auto key_matches(const char* key) -> bool;
  };

  using OAHTSlot = Slot;

  OAHashTable(const OAHTConfig& Config); // Constructor

  OAHashTable(OAHashTable&& from);

  OAHashTable(const OAHashTable& from);

  auto operator=(OAHashTable&& from) -> OAHashTable&;

  auto operator=(const OAHashTable& from) -> OAHashTable&;

  ~OAHashTable(); // Destructor

  // Insert a key/data pair into table. Throws an exception if the
  // insertion is unsuccessful.
  auto insert(const char* key, const T& data) -> void;

  // Delete an item by key. Throws an exception if the key doesn't exist.
  // Compacts the table by moving key/data pairs, if necessary
  auto remove(const char* key) -> void;

  // Find and return data by key. Throws an exception (E_ITEM_NOT_FOUND)
  // if not found.
  auto find(const char* key) const -> const T&;

  // Removes all items from the table (Doesn't deallocate table)
  auto clear() -> void;

  // Allow the client to peer into the data
  auto GetStats() const -> OAHTStats;

  auto GetTable() const -> const Slot*;

private: // Some suggestions (You don't have to use any of this.)

  // Initialize the table after an allocation
  auto init_table() -> void;

  // Expands the table when the load factor reaches a certain point
  // (greater than MaxLoadFactor) Grows the table by GrowthFactor,
  // making sure the new size is prime by calling GetClosestPrime
  auto grow() -> void;

  struct index_res {
    Slot* slot{nullptr};
    usize index{0};

    explicit operator bool() const { return slot != nullptr; }
  };

  // Workhorse method to locate an item (if it exists)
  // Returns the index of the item in the table
  // Sets Slot to point to the slot in the table where it belongs
  // Returns -1 if it's not in the table
  auto index_of(const char* key) const -> index_res;

  auto destruct() -> void;

  auto primary_hash(const char* key) -> usize;

  auto secondary_hash(const char* key) -> usize;

  mutable OAHTStats stats{};
  OAHTConfig config{};
  std::unique_ptr<OAHTSlot[]> slots{};
  usize capacity{};
  usize size{0};
};

#include "OAHashTable.cpp"

#endif
