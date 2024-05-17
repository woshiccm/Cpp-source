//
//  GCBitset.hpp
//  位运算
//
//  Created by Roy Cao on 2024/3/24.
//

#ifndef GCBitset_hpp
#define GCBitset_hpp

#include <stdio.h>
#include <cassert>
#include <climits>
#include <cstdint>

#define panda_bit_utils_ctz __builtin_ctz      // NOLINT(cppcoreguidelines-macro-usage)
#define panda_bit_utils_ctzll __builtin_ctzll  // NOLINT(cppcoreguidelines-macro-usage)

class MathHelper {
public:
  static constexpr uint32_t GetIntLog2(const uint32_t X) {
    return static_cast<uint32_t>(panda_bit_utils_ctz(X));
  }
  
  static constexpr uint64_t GetIntLog2(const uint64_t X) {
    return static_cast<uint64_t>(panda_bit_utils_ctzll(X));
  }
};

// |----word(32 bit)----|----word(32 bit)----|----...----|----word(32 bit)----|----word(32 bit)----|
// |---------------------------------------GCBitset(4 kb)------------------------------------------|

enum class AccessType { ATOMIC, NON_ATOMIC };

class GCBitset {
public:
  using GCBitsetWord = uint32_t;
  static constexpr uint32_t BYTE_PER_WORD = sizeof(GCBitsetWord);
  static constexpr uint32_t BYTE_PER_WORD_LOG2 = MathHelper::GetIntLog2(BYTE_PER_WORD);
  static constexpr uint32_t BIT_PER_BYTE = 8;
  static constexpr uint32_t BIT_PER_BYTE_LOG2 = MathHelper::GetIntLog2(BIT_PER_BYTE);
  static constexpr uint32_t BIT_PER_WORD = BYTE_PER_WORD * BIT_PER_BYTE;
  static constexpr uint32_t BIT_PER_WORD_LOG2 = MathHelper::GetIntLog2(BIT_PER_WORD);
  static constexpr uint32_t BIT_PER_WORD_MASK = BIT_PER_WORD - 1;
  
  GCBitset() = default;
  ~GCBitset() = default;
  
  GCBitsetWord *Words() {
    return reinterpret_cast<GCBitsetWord *>(this);
  }

  const GCBitsetWord *Words() const {
    return reinterpret_cast<const GCBitsetWord *>(this);
  }
  
  // Only used for snapshot to record region index
  void SetGCWords(uint32_t index) {
    *reinterpret_cast<GCBitsetWord *>(this) = index;
  }

  void Clear(size_t bitSize) {
    GCBitsetWord *words = Words();
    uint32_t wordCount = static_cast<uint32_t>(WordCount(bitSize));
    for (uint32_t i = 0; i < wordCount; i++) {
      words[i] = 0;
    }
  }

  void SetAllBits(size_t bitSize) {
    GCBitsetWord *words = Words();
    uint32_t wordCount = static_cast<uint32_t>(WordCount(bitSize));
    GCBitsetWord mask = 0;
    for (uint32_t i = 0; i < wordCount; i++) {
      words[i] = ~mask;
    }
  }
  
  template <AccessType mode = AccessType::NON_ATOMIC>
  bool SetBit(uintptr_t offset);
  
  void ClearBit(uintptr_t offset) {
    Words()[Index(offset)] &= ~Mask(IndexInWord(offset));
  }
  
private:
  GCBitsetWord Mask(size_t index) const {
    return 1 << index;
  }

  size_t IndexInWord(uintptr_t offset) const {
    return offset & BIT_PER_WORD_MASK;
  }

  size_t Index(uintptr_t offset) const {
    return offset >> BIT_PER_WORD_LOG2;
  }

  size_t WordCount(size_t size) const {
    return size >> BYTE_PER_WORD_LOG2;
  }
  
};

template <>
inline bool GCBitset::SetBit<AccessType::NON_ATOMIC>(uintptr_t offset) {
  size_t index = Index(offset);
  GCBitsetWord mask = Mask(IndexInWord(offset));
  if (Words()[index] & mask) {
    return false;
  }
  Words()[index] |= mask;
  return true;
}

template <>
inline bool GCBitset::SetBit<AccessType::ATOMIC>(uintptr_t offset) {
  auto word = reinterpret_cast<std::atomic<GCBitsetWord> *>(&Words()[Index(offset)]);
  auto mask = Mask(IndexInWord(offset));
  auto oldValue = word->load(std::memory_order_relaxed);
  do {
      if (oldValue & mask) {
          return false;
      }
  } while (!word->compare_exchange_weak(oldValue, oldValue | mask, std::memory_order_seq_cst));
  return true;
}

#endif /* GCBitset_hpp */
