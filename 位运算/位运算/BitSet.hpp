//
//  BitSet.hpp
//  位运算
//
//  Created by Roy Cao on 2024/3/24.
//

#ifndef BitSet_hpp
#define BitSet_hpp

// JSC

#include <array>
#include <stdio.h>
#include <string.h>
#include <type_traits>
#include <cstdint>

#if !defined(ALWAYS_INLINE)
#define ALWAYS_INLINE inline
#endif

#define CPU(WTF_FEATURE) (defined WTF_CPU_##WTF_FEATURE  && WTF_CPU_##WTF_FEATURE)

#if CPU(REGISTER64)
using CPURegister = int64_t;
using UCPURegister = uint64_t;
#else
using CPURegister = int32_t;
using UCPURegister = uint32_t;
#endif


template<size_t size>
using BitSetWordType = std::conditional_t<(size <= 32 && sizeof(UCPURegister) > sizeof(uint32_t)), uint32_t, UCPURegister>;

template<size_t bitSetSize, typename PassedWordType = BitSetWordType<bitSetSize>>
class BitSet final {

public:
  using WordType = PassedWordType;

  static_assert(sizeof(WordType) <= sizeof(UCPURegister), "WordType must not be bigger than the CPU atomic word size");
  constexpr BitSet() = default;

  static constexpr size_t size() {
    return bitSetSize;
  }

  constexpr void set(size_t);
  constexpr void set(size_t, bool);
  constexpr void clear(size_t);
  constexpr void clearAll();
  constexpr void setAll();
  
  constexpr size_t findBit(size_t startIndex, bool value) const;
  
  
private:
  
  void cleanseLastWord();
  
  static constexpr unsigned wordSize = sizeof(WordType) * 8;
  static constexpr unsigned words = (bitSetSize + wordSize - 1) / wordSize;

  // the literal '1' is of type signed int.  We want to use an unsigned
  // version of the correct size when doing the calculations because if
  // WordType is larger than int, '1 << 31' will first be sign extended
  // and then casted to unsigned, meaning that set(31) when WordType is
  // a 64 bit unsigned int would give 0xffff8000
  static constexpr WordType one = 1;

  std::array<WordType, words> bits { };
  
};

template<size_t bitSetSize, typename WordType>
ALWAYS_INLINE constexpr void BitSet<bitSetSize, WordType>::set(size_t n) {
    bits[n / wordSize] |= (one << (n % wordSize));
}

template<size_t bitSetSize, typename WordType>
ALWAYS_INLINE constexpr void BitSet<bitSetSize, WordType>::set(size_t n, bool value) {
  if (value)
    set(n);
  else
    clear(n);
}

template<size_t bitSetSize, typename WordType>
inline constexpr void BitSet<bitSetSize, WordType>::clear(size_t n) {
  bits[n / wordSize] &= ~(one << (n % wordSize));
}

template<size_t bitSetSize, typename WordType>
inline constexpr void BitSet<bitSetSize, WordType>::clearAll() {
  memset(bits.data(), 0, sizeof(bits));
}

template<size_t bitSetSize, typename WordType>
inline void BitSet<bitSetSize, WordType>::cleanseLastWord() {
  if constexpr (!!(bitSetSize % wordSize)) {
    constexpr size_t remainingBits = bitSetSize % wordSize;
    constexpr WordType mask = (static_cast<WordType>(1) << remainingBits) - 1;
    bits[words - 1] &= mask;
  }
}

template<size_t bitSetSize, typename WordType>
inline constexpr void BitSet<bitSetSize, WordType>::setAll() {
  memset(bits.data(), 0xFF, sizeof(bits));
  cleanseLastWord();
}

template<size_t bitSetSize, typename WordType>
inline constexpr size_t BitSet<bitSetSize, WordType>::findBit(size_t startIndex, bool value) const {
  WordType skipValue = -(static_cast<WordType>(value) ^ 1);
  size_t wordIndex = startIndex / wordSize;
  size_t startIndexInWord = startIndex - wordIndex * wordSize;
  
  while (wordIndex < words) {
    WordType word = bits[wordIndex];
    if (word != skipValue) {
      size_t index = startIndexInWord;
      if (findBitInWord(word, index, wordSize, value))
          return wordIndex * wordSize + index;
    }
    
    wordIndex++;
    startIndexInWord = 0;
  }
  
  return bitSetSize;
}

#endif /* BitSet_hpp */
