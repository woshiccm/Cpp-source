//
//  WordBitSet.hpp
//  位运算
//
//  Created by Roy Cao on 2024/3/24.
//

#ifndef WordBitSet_hpp
#define WordBitSet_hpp

#include <cassert>
#include <climits>
#include <cstdint>

// Hermes 0.5

/// A very simple bitset that fits in a single word of the specified type T.
template <typename T = uint32_t>
class WordBitSet {
  T value_{};

  enum { NUM_BITS = sizeof(T) * CHAR_BIT };

 public:
  WordBitSet() = default;
  WordBitSet(const WordBitSet &) = default;
  WordBitSet &operator=(const WordBitSet &) = default;
  ~WordBitSet() = default;

  /// \return true if no bits are set.
  bool empty() const {
    return value_ == 0;
  }

  /// Set bit at position \p pos to 1.
  WordBitSet &set(unsigned pos) {
    value_ |= (T)1 << pos;
    return *this;
  }

  /// Set bit at position \p pos to 0;
  WordBitSet &clear(unsigned pos) {
    value_ &= ~((T)1 << pos);
    return *this;
  }

  /// Return bit at position \p pos.
  bool at(unsigned pos) const {
    assert(pos < NUM_BITS && "Invalid index");
    return (value_ & ((T)1 << pos));
  }

  /// Return bit at position \p pos.
  bool operator[](unsigned pos) const {
    assert(pos < NUM_BITS && "Invalid index");
    return (value_ & ((T)1 << pos));
  }
};

#endif /* WordBitSet_hpp */
