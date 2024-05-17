//
//  NBitVector.hpp
//  位运算
//
//  Created by Roy Cao on 2024/5/14.
//

#ifndef NBitVector_hpp
#define NBitVector_hpp

#include <cstdint>
#include <cstring>
#include <iterator>
#include <type_traits>
#include <vector>
#include <valarray>
#include <cstddef>

// https://github.com/doitsujin/dxvk/blob/6259e863921777dfbdff5f907cbcfb02ce700b99/src/util/util_bit.h#L343

/// The behavior an operation has on an input of 0.
enum ZeroBehavior {
  /// The returned value is undefined.
  ZB_Undefined,
  /// The returned value is numeric_limits<T>::max()
  ZB_Max,
  /// The returned value is numeric_limits<T>::digits
  ZB_Width
};


template <typename T, std::size_t SizeOfT> struct TrailingZerosCounter {
  static std::size_t count(T Val, ZeroBehavior) {
    if (!Val)
      return std::numeric_limits<T>::digits;
    if (Val & 0x1)
      return 0;

    // Bisection method.
    std::size_t ZeroBits = 0;
    T Shift = std::numeric_limits<T>::digits >> 1;
    T Mask = std::numeric_limits<T>::max() >> Shift;
    while (Shift) {
      if ((Val & Mask) == 0) {
        Val >>= Shift;
        ZeroBits |= Shift;
      }
      Shift >>= 1;
      Mask >>= Shift;
    }
    return ZeroBits;
  }
};

/// Count number of 0's from the least significant bit to the most
///   stopping at the first 1.
///
/// Only unsigned integral types are allowed.
///
/// \param ZB the behavior on an input of 0. Only ZB_Width and ZB_Undefined are
///   valid arguments.
template <typename T>
std::size_t countTrailingZeros(T Val, ZeroBehavior ZB = ZB_Width) {
  static_assert(std::numeric_limits<T>::is_integer &&
                    !std::numeric_limits<T>::is_signed,
                "Only unsigned integral types are allowed.");
  return TrailingZerosCounter<T, sizeof(T)>::count(Val, ZB);
}


/// Create a bitmask with the N right-most bits set to 1, and all other
/// bits set to 0.  Only unsigned types are allowed.
template <typename T> T maskTrailingOnes(unsigned N) {
  static_assert(std::is_unsigned<T>::value, "Invalid type!");
  const unsigned Bits = CHAR_BIT * sizeof(T);
  assert(N <= Bits && "Invalid bit index");
  return N == 0 ? 0 : (T(-1) >> (Bits - N));
}

/// Create a bitmask with the N left-most bits set to 1, and all other
/// bits set to 0.  Only unsigned types are allowed.
template <typename T> T maskLeadingOnes(unsigned N) {
  return ~maskTrailingOnes<T>(CHAR_BIT * sizeof(T) - N);
}

/// Create a bitmask with the N right-most bits set to 0, and all other
/// bits set to 1.  Only unsigned types are allowed.
template <typename T> T maskTrailingZeros(unsigned N) {
  return maskLeadingOnes<T>(CHAR_BIT * sizeof(T) - N);
}

class NBitVector {
  
  typedef unsigned long BitWord;

  enum { BITWORD_SIZE = (unsigned)sizeof(BitWord) * CHAR_BIT };
  
  std::vector<BitWord> Bits;
  unsigned Size;
  
private:
  void init_words(std::vector<BitWord> &B, bool t) {
    if (B.size() > 0)
      memset(B.data(), 0 - (int)t, B.size() * sizeof(BitWord));
  }
  
  void init_words_from(std::vector<BitWord> &B, unsigned words, bool t) {
    for (size_t i = words * sizeof(BitWord); i < Bits.size() * sizeof(BitWord); i++)
      Bits.data()[i] = 0 - (int)t;
  }
  
  unsigned NumBitWords(unsigned S) const {
    return (S + BITWORD_SIZE - 1) / BITWORD_SIZE;
  }
  
public:
  typedef unsigned size_type;
  
  class reference {
    friend class BitVector;

    BitWord *WordRef;
    unsigned BitPos;

  public:
    reference(NBitVector &b, unsigned idx) {
      WordRef = &b.Bits[idx / BITWORD_SIZE];
      BitPos = idx % BITWORD_SIZE;
    }

    reference() = delete;
    reference(const reference&) = default;

    reference &operator=(reference t) {
      *this = bool(t);
      return *this;
    }

    reference& operator=(bool t) {
      if (t)
        *WordRef |= BitWord(1) << BitPos;
      else
        *WordRef &= ~(BitWord(1) << BitPos);
      return *this;
    }

    operator bool() const {
      return ((*WordRef) & (BitWord(1) << BitPos)) != 0;
    }
  };
  
  /// BitVector default ctor - Creates an empty bitvector.
  NBitVector() : Size(0) {}

  /// BitVector ctor - Creates a bitvector of specified number of bits. All
  /// bits are initialized to the specified value.
  explicit NBitVector(unsigned s, bool t = false) : Size(s) {
    size_t Capacity = NumBitWords(s);
    Bits.resize(Capacity, 0 - BitWord(t));
    init_words(Bits, t);
    if (t)
      clear_unused_bits();
  }
  
  bool empty() const { return Size == 0; }
  
  size_type size() const { return Size; }
  
  /// any - Returns true if any bit is set.
  bool any() const {
    for (unsigned i = 0; i < NumBitWords(size()); ++i)
      if (Bits[i] != 0)
        return true;
    return false;
  }
  
  /// all - Returns true if all bits are set.
  bool all() const {
    for (unsigned i = 0; i < Size / BITWORD_SIZE; ++i)
      if (Bits[i] != ~BitWord(0))
        return false;

    // If bits remain check that they are ones. The unused bits are always zero.
    if (unsigned Remainder = Size % BITWORD_SIZE)
      return Bits[Size / BITWORD_SIZE] == (BitWord(1) << Remainder) - 1;

    return true;
  }
  
  /// none - Returns true if none of the bits are set.
  bool none() const {
    return !any();
  }
  
  /// find_first_in - Returns the index of the first set bit in the range
  /// [Begin, End).  Returns -1 if all bits in the range are unset.
  int find_first_in(unsigned Begin, unsigned End) const {
    assert(Begin <= End && End <= Size);
    if (Begin == End)
      return -1;

    unsigned FirstWord = Begin / BITWORD_SIZE;
    unsigned LastWord = (End - 1) / BITWORD_SIZE;
    
    // Check subsequent words.
    for (unsigned i = FirstWord; i <= LastWord; ++i) {
      BitWord Copy = Bits[i];
      
      if (i == FirstWord) {
        unsigned FirstBit = Begin % BITWORD_SIZE;
        Copy &= maskTrailingZeros<BitWord>(FirstBit);
      }
      
      if (i == LastWord) {
        unsigned LastBit = (End - 1) % BITWORD_SIZE;
        Copy &= maskTrailingOnes<BitWord>(LastBit + 1);
      }
      if (Copy != 0)
        return i * BITWORD_SIZE + countTrailingZeros(Copy);
    }
    
    return -1;
  }
  
  /// find_first - Returns the index of the first set bit, -1 if none
  /// of the bits are set.
  int find_first() const { return find_first_in(0, Size); }

  bool at(uint32_t idx) const {
    assert(idx < Size && "Index must be within the bitset");
    return Bits[idx / BITWORD_SIZE] & BitWord(1) << (idx % BITWORD_SIZE);
  }
  
  /// clear - Removes all bits from the bitvector. Does not change capacity.
  void clear() {
    Size = 0;
  }
  
  /// resize - Grow or shrink the bitvector.
  void resize(unsigned N, bool t = false) {
    if (N > getBitCapacity()) {
      unsigned OldCapacity = Bits.size();
      Bits.resize(NumBitWords(N), 0 - BitWord(t));
      init_words_from(Bits, OldCapacity, t);
    }
    
    // Set any old unused bits that are now included in the BitVector. This
    // may set bits that are not included in the new vector, but we will clear
    // them back out below.
    if (N > Size)
      set_unused_bits(t);
    
    // Update the size, and clear out any bits that are now unused
    unsigned OldSize = Size;
    Size = N;
    if (t || N < OldSize)
      clear_unused_bits();
  }
  
  void reserve(unsigned N) {
    if (N > getBitCapacity())
      Bits.resize(NumBitWords(N), 0 - BitWord(false));
  }
  
  NBitVector &set() {
    init_words(Bits, true);
    return *this;
  }

  NBitVector &set(uint32_t idx) {
    Bits[idx / BITWORD_SIZE] |= BitWord(1) << (idx % BITWORD_SIZE);
    return *this;
  }
  
  NBitVector &reset() {
    init_words(Bits, false);
    return *this;
  }
  
  NBitVector &reset(unsigned idx) {
    Bits[idx / BITWORD_SIZE] &= ~(BitWord(1) << (idx % BITWORD_SIZE));
    return *this;
  }
  
  NBitVector &flip(unsigned idx) {
    Bits[idx / BITWORD_SIZE] ^= BitWord(1) << (idx % BITWORD_SIZE);
    return *this;
  }
  
  reference operator[](unsigned idx) {
    assert (idx < Size && "Out-of-bounds Bit access.");
    return reference(*this, idx);
  }

  bool operator[](unsigned idx) const {
    assert (idx < Size && "Out-of-bounds Bit access.");
    BitWord Mask = BitWord(1) << (idx % BITWORD_SIZE);
    return (Bits[idx / BITWORD_SIZE] & Mask) != 0;
  }
  
  bool test(unsigned idx) const {
    return (*this)[idx];
  }

  // Set the unused bits in the high words.
  void set_unused_bits(bool t = true) {
    //  Set high words first.
    unsigned UsedWords = NumBitWords(Size);
    if (Bits.size() > UsedWords)
      init_words_from(Bits, UsedWords, t);

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
  
public:
  /// Return the size (in bytes) of the bit vector.
  size_t getMemorySize() const { return Bits.size() * sizeof(BitWord); }
  size_t getBitCapacity() const { return Bits.size() * BITWORD_SIZE; }
  
};

#endif /* NBitVector_hpp */
