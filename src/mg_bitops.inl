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
  return (V == 0) ? -1 : i8(mg_BitSizeOf(V) - 1 - __builtin_clz(V));
}
mg_Inline constexpr i8
Msb(u64 V) {
  return (V == 0) ? -1 : i8(mg_BitSizeOf(V) - 1 -__builtin_clzll(V));
}
mg_Inline constexpr i8
Lsb(u32 V, i8 Default) {
  return (V == 0) ? Default : i8(__builtin_ctz(V));
}
mg_Inline constexpr i8
Lsb(u64 V, i8 Default) {
  return (V == 0) ? Default : i8(__builtin_ctzll(V));
}
#elif defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanReverse64)
mg_Inline constexpr i8
Msb(u32 V, i8 Default) {
  unsigned long Index = 0;
  unsigned char Ret = _BitScanReverse(&Index, V);
  return Ret ? (i8)Index : Default;
}
mg_Inline constexpr i8
Msb(u64 V, i8 Default) {
  unsigned long Index = 0;
  unsigned char _BitScanReverse64(&Index, V);
  return Ret ? (i8)Index : Default;
}
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)
mg_Inline constexpr i8
Lsb(u32 V) {
  unsigned long Index = 0;
  unsigned char Ret = _BitScanForward(&Index, V);
  return Ret ? (i8)Index : -1;
}
mg_Inline constexpr i8
Lsb(u64 V) {
  unsigned long Index = 0;
  unsigned char Ret = _BitScanForward64(&Index, V);
  return Ret ? (i8)Index : -1;
}
#endif

// TODO: the following clashes with stlab which brings in MSVC's intrin.h
//#if defined(__BMI2__)
//#if defined(__clang__) || defined(__GNUC__)
//#include <intrin.h>
//#include <mmintrin.h>
//#include <x86intrin.h>
//mg_Inline i8
//LzCnt(u32 V, i8 Default) { return V ? (i8)_lzcnt_u32(V) : Default; }
//mg_Inline i8
//LzCnt(u64 V, i8 Default) { return V ? (i8)_lzcnt_u64(V) : Default; }
//mg_Inline i8
//TzCnt(u32 V, i8 Default) { return V ? (i8)_tzcnt_u32(V) : Default; }
//mg_Inline i8
//TzCnt(u64 V, i8 Default) { return V ? (i8)_tzcnt_u64(V) : Default; }
//#elif defined(_MSC_VER)
//#include <intrin.h>
//mg_Inline i8
//LzCnt(u32 V, i8 Default) { return V ? (i8)__lzcnt(V) : Default; }
//mg_Inline i8
//LzCnt(u64 V, i8 Default) { return V ? (i8)__lzcnt64(V) : Default; }
//mg_Inline i8
//TzCnt(u32 V, i8 Default) { return V ? (i8)__tzcnt(V) : Default; }
//mg_Inline i8
//TzCnt(u64 V, i8 Default) { return V ? (i8)__tzcnt64(V) : Default; }
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
Pack3i32(const v3i& V) { return u32(V.X & 0x3FF) + (u32(V.Y & 0x3FF) << 10) + (u32(V.Z & 0x3FF) << 20); }
mg_Inline v3i
Unpack3i32(u32 V) {
  return v3i((i32(V & 0x3FF) << 22) >> 22, (i32(V & 0xFFC00) << 12) >> 22, (i32(V & 0x3FFFFC00) << 2) >> 22);
}

mg_Inline u64
Pack3i64(const v3i& V) { return u64(V.X & 0x1FFFFF) + (u64(V.Y & 0x1FFFFF) << 21) + (u64(V.Z & 0x1FFFFF) << 42); }
mg_Inline v3i
Unpack3i64(u64 V) {
  return v3i((i64(V & 0x1FFFFF) << 43) >> 43, (i64(V & 0x3FFFFE00000) << 22) >> 43, (i64(V & 0x7FFFFC0000000000ull) << 1) >> 43);
}

mg_Inline u32
LowBits64(u64 V) { return V & 0xFFFFFFFF; }
mg_Inline u32
HighBits64(u64 V) { return V >> 32; }

inline int 
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

