//
//  MBitArray.hpp
//  位运算
//
//  Created by Roy Cao on 2024/3/25.
//

// mozilla

// https://github.com/mozilla/gecko-dev/blob/2537e5eaafaab9a7ef6be5cfc8e9b1e2c747fdfd/js/src/ds/BitArray.h#L38

#ifndef MBitArray_hpp
#define MBitArray_hpp

#include <stdio.h>

#include <limits.h>
#include <string.h>
#include <cstdint>
#include <cstddef>

template <size_t nbits>
class BitArray {
public:
  // Use a 32 bit word to make it easier to access a BitArray from JIT code.
  using WordT = uint32_t;
  
  static const size_t bitsPerElement = sizeof(WordT) * CHAR_BIT;
    static const size_t numSlots =
        nbits / bitsPerElement + (nbits % bitsPerElement == 0 ? 0 : 1);
  
private:
  static const size_t paddingBits = (numSlots * bitsPerElement) - nbits;
  static_assert(paddingBits < bitsPerElement,
                "More padding bits than expected.");
  static const WordT paddingMask = WordT(-1) >> paddingBits;

  WordT map[numSlots];
  
public:
  constexpr BitArray() : map(){};

  void clear(bool value) {
    memset(map, value ? 0xFF : 0, sizeof(map));
    if (value) {
      map[numSlots - 1] &= paddingMask;
    }
  }
  
  inline bool get(size_t offset) const {
    size_t index;
    WordT mask;
    getIndexAndMask(offset, &index, &mask);
    return map[index] & mask;
  }
  
  void set(size_t offset) {
      size_t index;
      WordT mask;
      getIndexAndMask(offset, &index, &mask);
      map[index] |= mask;
    }

    void unset(size_t offset) {
      size_t index;
      WordT mask;
      getIndexAndMask(offset, &index, &mask);
      map[index] &= ~mask;
    }

    bool isAllClear() const {
      for (size_t i = 0; i < numSlots; i++) {
        if (map[i]) {
          return false;
        }
      }
      return true;
    }
  
  // For iterating over the set bits in the bit array, get a word at a time.
    WordT getWord(size_t elementIndex) const {
      return map[elementIndex];
    }

    static void getIndexAndMask(size_t offset, size_t* indexp, WordT* maskp) {
      static_assert(bitsPerElement == 32, "unexpected bitsPerElement value");
      *indexp = offset / bitsPerElement;
      *maskp = WordT(1) << (offset % bitsPerElement);
    }

    static size_t offsetOfMap() {
      return offsetof(BitArray<nbits>, map);
    }
  
};

#endif /* MBitArray_hpp */
