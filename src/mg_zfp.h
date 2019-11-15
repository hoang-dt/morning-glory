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
mg_TI(t, S) void ForwardZfp2D(t* P); // TODO: for now this works only for S = 4
mg_TI(t, S) void InverseZfp2D(t* P); // TODO: for now this works only for S = 4

/* Reorder coefficients within a zfp block, and convert them from/to negabinary */
mg_TT(t, u) void ForwardShuffle(t* IBlock, u* UBlock);
mg_TT(t, u) void InverseShuffle(u* UBlock, t* IBlock);
mg_TTI(t, u, S) void ForwardShuffle2D(t* IBlock, u* UBlock);
mg_TTI(t, u, S) void InverseShuffle2D(u* UBlock, t* IBlock);

/* Pad partial block of width N < 4 and stride S */
mg_T(t) void PadBlock1D(t* P, int N, int S);
mg_T(t) void PadBlock2D(t* P, const v2i& N);
mg_T(t) void PadBlock3D(t* P, const v3i& N);

struct bitstream;
/* Encode/decode a single bit plane B of a zfp block */
// TODO: turn this into a template? TODO: pointer aliasing?
/*
B = the bit plane to encode
S = maximum number of bits to encode in the current pass
N keeps track of the number of coefficients that have previously become significant
Sometimes the stream is interrupted in the middle of a bit plane, so M keeps track
of the number of significant coefficients encoded in the current bit plane */
bool Encode(u64* Block, int B, i64 S, i8& N, i8& M, bool& In, bitstream* Bs);
bool Decode(u64* Block, int B, i64 S, i8& N, i8& M, bool& In, bitstream* Bs);
bool Encode(u64* Block, int B, i64 S, i8& N, i8& M, bitstream* Bs);
bool Decode(u64* Block, int B, i64 S, i8& N, i8& M, bitstream* Bs);
mg_TII(t, D = 3, K = 4) void Encode(t* Block, int B, i64 S, i8& N, bitstream* Bs);
mg_TII(t, D = 3, K = 4) void Decode(t* Block, int B, i64 S, i8& N, bitstream* Bs);
mg_TII(t, D = 3, K = 4) void Decode2(t* Block, int B, i64 S, i8& N, bitstream* Bs);
mg_TII(t, D = 3, K = 4) void Decode3(t* Block, int B, i64 S, i8& N, bitstream* Bs);
mg_TII(t, D = 3, K = 4) void Decode4(t* Block, int B, i64 S, i8& N, bitstream* Bs);

} // namespace mg

#include "mg_zfp.inl"
