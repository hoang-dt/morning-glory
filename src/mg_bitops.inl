#pragma once

#include "mg_assert.h"
#include "bitmap_tables.h"
#include <x86intrin.h>

namespace mg {

mg_Ti(t) t
SetBit(t Val, int I) {
  mg_Assert(I < (int)mg_BitSizeOf(t));
  return Val | t((1ull << I));
}

mg_Ti(t) t
UnsetBit(t Val, int I) {
  mg_Assert(I < (int)mg_BitSizeOf(t));
  return Val & t(~(1ull << I));
}

mg_Ti(t) bool
BitSet(t Val, int I) {
  mg_Assert(I < (int)mg_BitSizeOf(t));
  return 1 & (Val >> I);
}

mg_Ti(t) t
FlipBit(t Val, int I) {
  mg_Assert(I < (int)mg_BitSizeOf(t));
  return Val ^ t(1ull << I);
}

// TODO: replace bsr with the faster intrinsic

#if defined(__clang__) || defined(__GNUC__)
mg_Inline constexpr i8
Msb(u32 V) {
  if (V == 0) return -1;
  return i8(mg_BitSizeOf(V) - 1 - __builtin_clz(V));
}
mg_Inline constexpr i8
Msb(u64 V) {
  if (V == 0) return -1;
  return i8(mg_BitSizeOf(V) - 1 -__builtin_clzll(V));
}
#elif defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanReverse64)
mg_Inline constexpr i8
Msb(u32 V) {
  if (V == 0) return -1;
  unsigned long Index = 0;
  _BitScanReverse(&Index, V);
  return (i8)Index;
}
mg_Inline constexpr i8
Msb(u64 V) {
  if (V == 0) return -1;
  unsigned long Index = 0;
  _BitScanReverse64(&Index, V);
  return (i8)Index;
}
#endif

// TODO: the following clashes with stlab which brings in MSVC's intrin.h
//#if defined(__BMI2__)
//#if defined(__clang__) || defined(__GNUC__)
//#include <intrin.h>
//#include <mmintrin.h>
//#include <x86intrin.h>
//mg_Inline i8
//LzCnt(u32 V) { return (i8)_lzcnt_u32(V); }
//mg_Inline i8
//LzCnt(u64 V) { return (i8)_lzcnt_u64(V); }
//#elif defined(_MSC_VER)
//#include <intrin.h>
//mg_Inline i8
//LzCnt(u32 V) { return (i8)__lzcnt(V); }
//mg_Inline i8
//LzCnt(u64 V) { return (i8)__lzcnt64(V); }
//#endif
//#endif

/* Reverse the operation that inserts two 0 bits after every bit of x */
mg_Inline u32
CompactBy2(u32 X) {
  X &= 0x09249249;                  // X = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  X = (X ^ (X >>  2)) & 0x030c30c3; // X = ---- --98 ---- 76-- --54 ---- 32-- --10
  X = (X ^ (X >>  4)) & 0x0300f00f; // X = ---- --98 ---- ---- 7654 ---- ---- 3210
  X = (X ^ (X >>  8)) & 0x030000ff; // X = ---- --98 ---- ---- ---- ---- 7654 3210
  X = (X ^ (X >> 16)) & 0x000003ff; // X = ---- ---- ---- ---- ---- --98 7654 3210
  return X;
}

mg_Inline u32
CompactBy1(u32 X) {
  X &= 0x55555555;                 // X = -5-4 -3-2 -1-0 -9-8 -7-6 -5-4 -3-2 -1-0
  X = (X ^ (X >> 1)) & 0x33333333; // X = --54 --32 --10 --98 --76 --54 --32 --10
  X = (X ^ (X >> 2)) & 0x0f0f0f0f; // X = ---- 5432 ---- 1098 ---- 7654 ---- 3210
  X = (X ^ (X >> 4)) & 0x00ff00ff; // X = ---- ---- 5432 1098 ---- ---- 7654 3210
  X = (X ^ (X >> 8)) & 0x0000ffff; // X = ---- ---- ---- ---- 5432 1098 7654 3210
  return X;
}

/* Morton decoding */
mg_Inline u32
DecodeMorton3X(u32 Code) { return CompactBy2(Code >> 0); }
mg_Inline u32
DecodeMorton3Y(u32 Code) { return CompactBy2(Code >> 1); }
mg_Inline u32
DecodeMorton3Z(u32 Code) { return CompactBy2(Code >> 2); }
mg_Inline u32
DecodeMorton2X(u32 Code) { return CompactBy1(Code >> 0); }
mg_Inline u32
DecodeMorton2Y(u32 Code) { return CompactBy1(Code >> 1); }

mg_Inline u32
SplitBy2(u32 X) {
  X &= 0x000003ff;                  // X = ---- ---- ---- ---- ---- --98 7654 3210
  X = (X ^ (X << 16)) & 0x030000ff; // X = ---- --98 ---- ---- ---- ---- 7654 3210
  X = (X ^ (X <<  8)) & 0x0300f00f; // X = ---- --98 ---- ---- 7654 ---- ---- 3210
  X = (X ^ (X <<  4)) & 0x030c30c3; // X = ---- --98 ---- 76-- --54 ---- 32-- --10
  X = (X ^ (X <<  2)) & 0x09249249; // X = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  return X;
}

mg_Inline u32
EncodeMorton3(u32 X, u32 Y, u32 Z) {
  return SplitBy2(X) | (SplitBy2(Y) << 1) | (SplitBy2(Z) << 2);
}

mg_Inline u32
Pack3Ints32(v3i V) { return u32(V.X) + (u32(V.Y) << 10) + (u32(V.Z) << 20); }
mg_Inline v3i
Unpack3Ints32(u32 V) {
  return v3i(V & 0x3FF, (V & 0xFFC00) >> 10, (V & 0x3FFFFC00) >> 20);
}

mg_Inline u64
Pack3i64(v3i V) { return u64(V.X) + (u64(V.Y) << 21) + (u64(V.Z) << 42); }
mg_Inline v3i
Unpack3i64(u64 V) {
  return v3i(V & 0x1FFFFF, (V & 0x3FFFFE00000) >> 21, (V & 0x7FFFFC0000000000ull) >> 42);
}

mg_Inline u32
LowBits64(u64 V) { return V & 0xFFFFFFFF; }
mg_Inline u32
HighBits64(u64 V) { return V >> 32; }

mg_Inline int 
DecodeBitmap(u64 Val, int* Out) {
  int* OutBackup = Out;
  __m256i BaseVec = _mm256_set1_epi32(-1);
  __m256i IncVec = _mm256_set1_epi32(64);
  __m256i Add8 = _mm256_set1_epi32(8);

  if (Val == 0) {
    BaseVec = _mm256_add_epi32(BaseVec, IncVec);
  } else {
    for (int K = 0; K < 4; ++K) {
      uint8_t ByteA = (uint8_t)Val;
      uint8_t ByteB = (uint8_t)(Val >> 8);
      Val >>= 16;
      __m256i VecA = _mm256_cvtepu8_epi32(_mm_cvtsi64_si128(*(uint64_t*)(vecDecodeTableByte[ByteA])));
      __m256i VecB = _mm256_cvtepu8_epi32(_mm_cvtsi64_si128(*(uint64_t*)(vecDecodeTableByte[ByteB])));
      uint8_t AdvanceA = lengthTable[ByteA];
      uint8_t AdvanceB = lengthTable[ByteB];
      VecA = _mm256_add_epi32(BaseVec, VecA);
      BaseVec = _mm256_add_epi32(BaseVec, Add8);
      VecB = _mm256_add_epi32(BaseVec, VecB);
      BaseVec = _mm256_add_epi32(BaseVec, Add8);
      _mm256_storeu_si256((__m256i*)Out, VecA);
      Out += AdvanceA;
      _mm256_storeu_si256((__m256i*)Out, VecB);
      Out += AdvanceB;
    }
  }
  return Out - OutBackup;
}
//
} // namespace mg

