#pragma once

#include "mg_assert.h"
#include "mg_macros.h"
#include "mg_types.h"

#define mg_BitSizeOf(X) (sizeof(X) * 8)

namespace mg {

/* Set the I(th) least significant bit of val to 1. Index starts at 0. */
template <typename t> t SetBit(t Val, int I);
/* Set the I(th) least significant bit of val to 0. Index starts at 0. */
template <typename t> t UnsetBit(t Val, int I);
/* Check if the I(th) least significant bit of val is 1. Index starts at 0. */
template <typename t> bool CheckBit(t Val, int I);
/* Flip the I(th) least significant bit of val. Index starts at 0. */
template <typename t> t FlipBit(t Val, int I);

/* Return the bit plane of the most significant one-bit. Counting starts from the least significant
bit plane. Examples: Bsr(0) = -1, Bsr(2) = 1, Bsr(5) = 2, Bsr(8) = 3 */
i8 BitScanReverse(u32 v);
i8 BitScanReverse(u64 V);

/* Morton encoding/decoding */
u32 DecodeMorton3X(u32 Code);
u32 DecodeMorton3Y(u32 Code);
u32 DecodeMorton3Z(u32 Code);
u32 EncodeMorton3(u32 X, u32 Y, u32 Z);

} // namespace mg

#include "mg_bitops.inl"
