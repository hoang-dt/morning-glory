/* Adapted from the zfp compression library */

#pragma once

#include "mg_bitstream.h"
#include "mg_types.h"

namespace mg {

extern const v3i ZfpBlockDims;

template <typename t>
void ForwardLift(t* P, int S);
template <typename t>
void InverseLift(t* P, int S);

/* zfp forward transform for 64 samples in 3D. The input is assumed to be in row-major order. */
template <typename t>
void ForwardBlockTransform(t* P);
/* zfp inverse transform for 64 samples in 3D. The input is assumed to be in row-major order. */
template <typename t>
void InverseBlockTransform(t* P);

/* Reorder signed coefficients within a zfp block, and convert them to negabinary */
template <typename t, typename u>
void ForwardShuffle(const t* IBlock, u* UBlock);

/* Reorder unsigned coefficients within a block, and convert them to two's complement */
template <typename t, typename u>
void InverseShuffle(const u* UBlock, t* IBlock);

/* Pad partial block of width N < 4 and stride S */
template <typename t>
void PadBlock(t* P, int N, int S);

/* Encode a single bit plane of a single zfp block */
// TODO: turn this into a template?
bool EncodeBlock(const u64* Block, int Bitplane, int BitsMax, i8& N, i8& M, 
                 bool& InnerLoop, bitstream* Bs);
/* Decode a single bit plane of a single zfp block */
// TODO: pointer aliasing?
bool DecodeBlock(u64* Block, int Bitplane, int BitsMax, i8& N, i8& M, 
                 bool& InnerLoop, bitstream* Bs);

} // namespace mg

#include "mg_zfp.inl"
