#pragma once

#include "mg_macros.h"
#include "mg_common.h"

namespace mg {

/* Set the I(th) least significant bit of val to 1. Index starts at 0. */
mg_T(t) t    SetBit  (t Val, int I);
mg_T(t) t    UnsetBit(t Val, int I);
mg_T(t) bool BitSet  (t Val, int I);
mg_T(t) t    FlipBit (t Val, int I);

/*
Return the bit plane of the most significant one-bit. Counting starts from the
least significant bit plane.
Examples: Msb(0) = -1, Msb(2) = 1, Msb(5) = 2, Msb(8) = 3 */
constexpr i8 Msb(u32 V);
constexpr i8 Msb(u64 V);
constexpr i8 Lsb(u32 V, i8 Default = -1);
constexpr i8 Lsb(u64 V, i8 Default = -1);
/* Count the number of leading zero bits */
i8 LzCnt(u32 V, i8 Default = -1);
i8 LzCnt(u64 V, i8 Default = -1);
i8 TzCnt(u32 V, i8 Default = -1);
i8 TzCnt(u64 V, i8 Default = -1);

/* Morton encoding/decoding */
u32 DecodeMorton3X(u32 Code);
u32 DecodeMorton3Y(u32 Code);
u32 DecodeMorton3Z(u32 Code);
u32 DecodeMorton2X(u32 Code);
u32 DecodeMorton2Y(u32 Code);
u32 EncodeMorton3(u32 X, u32 Y, u32 Z);

/* Stuff three 21-bit uints into one 64-bit uint */
u64 Pack3i64  (v3i V);
v3i Unpack3i64(u64 V);
/* Return the low 32 bits of the input */
u32 LowBits64 (u64 V);
u32 HighBits64(u64 V);

} // namespace mg

#include "mg_bitops.inl"
