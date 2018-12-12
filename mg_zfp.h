/* Adapted from the zfp compression library */

#pragma once

namespace mg {

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
struct bit_stream;
void EncodeBlock(const u64* Block, int Bitplane, int& N, bit_stream* Bs);
/* Decode a single bit plane of a single zfp block */
// TODO: pointer aliasing?
void DecodeBlock(u64* Block, int Bitplane, int& N, bit_stream* Bs);

struct block;
template <typename t> struct dynamic_array;
void EncodeData(const f64* Data, v3i Dims, v3i TileDims, const dynamic_array<Block>& Subbands, cstr FileName, bit_stream* Bs);
void DecodeData(f64* Data, v3i Dims, v3i TileDims);

} // namespace mg

#include "mg_zfp.inl"
