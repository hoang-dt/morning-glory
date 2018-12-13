#pragma once

#include "mg_assert.h"
#include "mg_macros.h"
#include "mg_types.h"

namespace mg {

/* Set the I(th) least significant bit of val to 1. Index starts at 0. */
template <typename t> mg_ForceInline
t SetBit(t Val, int I) {
  mg_Assert(I < mg_BitSizeOf(t));
  return Val | t((1ull << I));
}

/* Set the I(th) least significant bit of val to 0. Index starts at 0. */
template <typename t> mg_ForceInline
t UnsetBit(t Val, int I) {
  mg_Assert(I < mg_BitSizeOf(t));
  return Val & t(~(1ull << I));
}

/* Check if the I(th) least significant bit of val is 1. Index starts at 0. */
template <typename t> mg_ForceInline
bool CheckBit(t Val, int I) {
  mg_Assert(I < mg_BitSizeOf(t));
  return 1 & (Val >> I);
}

/* Flip the I(th) least significant bit of val. Index starts at 0. */
template <typename t> mg_ForceInline
t FlipBit(t Val, int I) {
  mg_Assert(I < mg_BitSizeOf(t));
  return Val ^ t(1ull << I);
}

/* Return the bit plane of the most significant one-bit. Counting starts from the least significant
bit plane. Examples: Bsr(0) = -1, Bsr(2) = 1, Bsr(5) = 2, Bsr(8) = 3 */
#if defined(__clang__) || defined(__GNUC__)
mg_ForceInline
i8 BitScanReverse(u32 V) {
  if (V == 0) return -1;
  return i8(mg_BitSizeOf(V) - 1 - __builtin_clz(V));
}
mg_ForceInline
i8 BitScanReverse(u64 V) {
  if (V == 0) return -1;
  return i8(mg_BitSizeOf(V) - 1 -__builtin_clzll(V));
}
#elif defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanReverse64)
mg_ForceInline
i8 BitScanReverse(u32 V) {
  if (V == 0) return -1;
  unsigned long Index = 0;
  _BitScanReverse(&Index, V);
  return (i8)Index;
}
mg_ForceInline
i8 BitScanReverse(u64 V) {
  if (V == 0) return -1;
  unsigned long Index = 0;
  _BitScanReverse64(&Index, V);
  return (i8)Index;
}
#endif

/* Reverse the operation that inserts two 0 bits after every bit of x */
mg_ForceInline
u32 CompactBy2(u32 X) {
  X &= 0x09249249;                  // X = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  X = (X ^ (X >>  2)) & 0x030c30c3; // X = ---- --98 ---- 76-- --54 ---- 32-- --10
  X = (X ^ (X >>  4)) & 0x0300f00f; // X = ---- --98 ---- ---- 7654 ---- ---- 3210
  X = (X ^ (X >>  8)) & 0x030000ff; // X = ---- --98 ---- ---- ---- ---- 7654 3210
  X = (X ^ (X >> 16)) & 0x000003ff; // X = ---- ---- ---- ---- ---- --98 7654 3210
  return X;
}

/* Morton decoding */
mg_ForceInline
u32 DecodeMorton3X(u32 Code) {
  return CompactBy2(Code >> 0);
}
mg_ForceInline
u32 DecodeMorton3Y(u32 Code) {
  return CompactBy2(Code >> 1);
}
mg_ForceInline
u32 DecodeMorton3Z(u32 Code) {
  return CompactBy2(Code >> 2);
}

mg_ForceInline
u32 SplitBy2(u32 X){
  X &= 0x000003ff;                  // X = ---- ---- ---- ---- ---- --98 7654 3210
  X = (X ^ (X << 16)) & 0x030000ff; // X = ---- --98 ---- ---- ---- ---- 7654 3210
  X = (X ^ (X <<  8)) & 0x0300f00f; // X = ---- --98 ---- ---- 7654 ---- ---- 3210
  X = (X ^ (X <<  4)) & 0x030c30c3; // X = ---- --98 ---- 76-- --54 ---- 32-- --10
  X = (X ^ (X <<  2)) & 0x09249249; // X = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  return X;
}

mg_ForceInline
u32 EncodeMorton3(u32 X, u32 Y, u32 Z) {
  return SplitBy2(X) | (SplitBy2(Y) << 1) | (SplitBy2(Z) << 2);
}

} // namespace mg
