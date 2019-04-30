#pragma once

#include "mg_assert.h"
#include "mg_macros.h"
#include "mg_types.h"

#define mg_BitSizeOf(X) ((int)sizeof(X) * 8)

namespace mg {

/* Set the I(th) least significant bit of val to 1. Index starts at 0. */
template <typename t> t SetBit(t Val, int I);
/* Set the I(th) least significant bit of val to 0. Index starts at 0. */
template <typename t> t UnsetBit(t Val, int I);
/* Check if the I(th) least significant bit of val is 1. Index starts at 0. */
template <typename t> bool CheckBit(t Val, int I);
/* Flip the I(th) least significant bit of val. Index starts at 0. */
template <typename t> t FlipBit(t Val, int I);

/* Return the bit plane of the most significant one-bit. Counting starts from the least
significant bit plane. Examples: Msb(0) = -1, Msb(2) = 1, Msb(5) = 2, Msb(8) = 3 */
i8 Msb(u32 v);
i8 Msb(u64 V);

/* Morton encoding/decoding */
u32 DecodeMorton3X(u32 Code);
u32 DecodeMorton3Y(u32 Code);
u32 DecodeMorton3Z(u32 Code);
u32 EncodeMorton3(u32 X, u32 Y, u32 Z);

/* Stuff three 21-bit uints into one 64-bit uint */
u64 Pack3Ints64(v3i V);
/* The inverse of Stuff3Ints */
v3i Unpack3Ints64(u64 V);
/* Return the low 32 bits of the input */
u32 LowBits64(u64 V);
u32 HighBits64(u64 V);

} // namespace mg

#include "mg_bitops.inl"
