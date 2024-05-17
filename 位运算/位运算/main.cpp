//
//  main.cpp
//  位运算
//
//  Created by Roy Cao on 2024/3/24.
//

// https://codeforces.com/blog/entry/77480
// https://codereview.stackexchange.com/questions/86623/c-bitset-implementation/86645#86645?newreg=69bc24700ea547e58c42fe8c929b0695

#include <iostream>
# include "BitVector.hpp"
# include "WordBitSet.hpp"
# include "GCBitset.hpp"
#include "BitSet.hpp"
#include "MBitArray.hpp"
#include "NBitVector.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <string>

/*
 & 与
 10010010
 00110100
 00010000
 
 | 或
 01010011
 01101011
 01111011
 
 ∧ 异或
 10010010
 11010100
 01000110
 
BitSet
 
1. 高效的增删改查（CPU级别的O(1)）
2. 高效的内存利用（例如将8个bool存在一个字节里）
 
 */

#define OPERATIN_SYSTEM   0x1   // 00000001
#define INTRO_PROGRAMMING 0x2   // 00000010
#define DATA_STRUCTURE    0x4   // 00000100
#define ALGORITHM         0x8   // 00001000
#define DATABASE_SYSTEM   0x10  // 00010000


void reset(std::vector<bool> &array) {
  memset(&array, false, 2);
}

#include <span>
#include <vector>
#include <iostream>

template<int left = 0, int right = 0, typename T>
constexpr auto slice(T&& container)
{
    if constexpr (right > 0)
    {
        return std::span(begin(std::forward<T>(container))+left, begin(std::forward<T>(container))+right);
    }
    else
    {
        return std::span(begin(std::forward<T>(container))+left, end(std::forward<T>(container))+right);
    }
}

int main(int argc, const char * argv[]) {
//  bool boolean[8]; // 8个字节
  
  // 最开始没有选课
  unsigned char course = 0; // 00000000
  
  // 选择DATA_STRUCTURE
  course |= DATA_STRUCTURE;  // 00000100
  course |= ALGORITHM;       // 00001100
  course |= DATABASE_SYSTEM; // 00011100
  
  // 查看有没有选择 ALGORITHM
  if ((course & ALGORITHM) == 0){
    printf("选择了 ALGORITHM");
  } else {
    printf("没有选择 ALGORITHM");
  }
  
  // 取消 DATABASE_SYSTEM
  course &= ~DATABASE_SYSTEM;
  // course            00011100
  // ~DATABASE_SYSTEM  11101111
  // &=                00001100
  
  // 为什么不用异或？
  
  auto a = std::numeric_limits<uintptr_t>::max();
  
  auto b = CHAR_BIT;
  
  typedef unsigned long BitWord;
  
  auto c = sizeof(BitWord);
  auto d = (unsigned)sizeof(BitWord);
  
  auto e = sizeof(uintptr_t);
  
  auto f = sizeof(uint32_t);
  
  size_t N = 4096 * 1024;
  
  const size_t ChunkShift = 20;
  const size_t ChunkSize = size_t(1) << ChunkShift;
  
  const size_t PageSize = size_t(1) << 12;
  

  auto m = N;

  NBitVector vector;
  vector.resize(10);
  
//  vector.set(0);
  vector.set(1);
  vector.set(3);
  
  bool v0 = vector.at(0);
  bool v1 = vector.at(1);
  bool v2 = vector.at(2);
  bool v3 = vector.at(3);
  
//  vector.reset(1);
  bool v11 = vector.at(1);
  auto bb = vector.find_first();
  
  std::vector<bool> vec;
  vec.push_back(true);
  vec.push_back(true);
  vec.push_back(true);
  vec.push_back(true);
  int size1 = vec.size();
  
//  vec.resize(7, 0 - uintptr_t(false));
  
//  reset(vec);
  
//  auto vec2 = slice<0, 3>(vec);
  
  std::vector<int> vecto{1,2,3,4,5,6,7,8,9};
  auto cd = vecto.data();
  
  return 0;
}

