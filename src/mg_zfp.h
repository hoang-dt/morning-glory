/* Adapted from the zfp compression library */

#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

extern const v3i ZDims; /* 4 x 4 x 4 */

/* Forward/inverse zfp lifting in 1D */
mg_T(t) void FLift(t* P, int S);
mg_T(t) void ILift(t* P, int S);

/* zfp transform in 3D. The input is assumed to be in row-major order. */
mg_T(t) void ForwardZfp(t* P);
mg_T(t) void InverseZfp(t* P);

/* Reorder coefficients within a zfp block, and convert them from/to negabinary */
mg_T2(t, u) void ForwardShuffle(t* IBlock, u* UBlock);
mg_T2(t, u) void InverseShuffle(u* UBlock, t* IBlock);

/* Pad partial block of width N < 4 and stride S */
mg_T(t) void PadBlock(t* P, int N, int S);

struct bitstream;
/* Encode/decode a single bit plane B of a zfp block */
// TODO: turn this into a template? TODO: pointer aliasing?
bool Encode(u64* Block, int B, int Bits, i8& N, i8& M, bool& In, bitstream* Bs);
bool Decode(u64* Block, int B, int Bits, i8& N, i8& M, bool& In, bitstream* Bs);
bool Encode(u64* Block, int B, int Bits, i8& N, i8& M, bitstream* Bs);
bool Decode(u64* Block, int B, int Bits, i8& N, i8& M, bitstream* Bs);

} // namespace mg

#include "mg_zfp.inl"
