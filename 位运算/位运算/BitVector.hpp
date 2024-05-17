//
//  BitVector.hpp
//  位运算
//
//  Created by Roy Cao on 2024/3/24.
//

#ifndef BitVector_hpp
#define BitVector_hpp

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>

// Hermes 0.5

class BitVector {
  typedef unsigned long BitWord;
  
  enum { BITWORD_SIZE = (unsigned)sizeof(BitWord) * CHAR_BIT };
  
  static_assert(BITWORD_SIZE == 64 || BITWORD_SIZE == 32,
                "Unsupported word size");
  
  std::vector<BitWord> Bits; // Actual bits.
  unsigned Size;                 // Size of bitvector in bits.
  
public:
  typedef unsigned size_type;
  
  BitVector() : Size(0) {}
  
  ~BitVector() { std::free(Bits.data()); }
  
  bool empty() const { return Size == 0; }
  
  /// size - Returns the number of bits in this bitvector.
  size_type size() const { return Size; }
  
  void init_words(std::vector<BitWord> B, bool t) {
    if (B.size() > 0)
      memset(B.data(), 0 - (int)t, B.size() * sizeof(BitWord));
  }
  
  unsigned NumBitWords(unsigned S) const {
    return (S + BITWORD_SIZE-1) / BITWORD_SIZE;
  }
  
  // Set the unused bits in the high words.
  void set_unused_bits(bool t = true) {
    //  Set high words first.
    unsigned UsedWords = NumBitWords(Size);
    if (Bits.size() > UsedWords) {
//      init_words(Bits.drop_front(UsedWords), t);
    }

    //  Then set any stray high bits of the last used word.
    unsigned ExtraBits = Size % BITWORD_SIZE;
    if (ExtraBits) {
      BitWord ExtraBitMask = ~0UL << ExtraBits;
      if (t)
        Bits[UsedWords-1] |= ExtraBitMask;
      else
        Bits[UsedWords-1] &= ~ExtraBitMask;
    }
  }

  // Clear the unused bits in the high words.
  void clear_unused_bits() {
    set_unused_bits(false);
  }
  
  BitVector &reset() {
    init_words(Bits, false);
    return *this;
  }

  BitVector &reset(unsigned Idx) {
    Bits[Idx / BITWORD_SIZE] &= ~(BitWord(1) << (Idx % BITWORD_SIZE));
    return *this;
  }
  
  // Set, reset, flip
  BitVector &set() {
    init_words(Bits, true);
    clear_unused_bits();
    return *this;
  }

  BitVector &set(unsigned Idx) {
    assert(Bits.data() && "Bits never allocated");
    Bits[Idx / BITWORD_SIZE] |= BitWord(1) << (Idx % BITWORD_SIZE);
    return *this;
  }
  
  
  
};

#endif /* BitVector_hpp */
