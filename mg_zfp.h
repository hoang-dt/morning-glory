/* Adapted from the zfp compression library */

#pragma once

#include "mg_bitstream.h"
#include "mg_common_types.h"
#include "mg_linked_list.h"
#include "mg_types.h"
#include "mg_volume.h"

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
void EncodeBlock(const u64* Block, int Bitplane, int& N, bitstream* Bs);
/* Decode a single bit plane of a single zfp block */
// TODO: pointer aliasing?
void DecodeBlock(u64* Block, int Bitplane, int& N, bitstream* Bs);

template <typename t> struct dynamic_array;
void EncodeData(const volume& Vol, v3i TileDims, int Bits, f64 Tolerance,
                const dynamic_array<extent>& Subbands, cstr FileName);
void DecodeData(volume* Vol, v3i TileDims, int Bits, f64 Tolerance,
                const dynamic_array<extent>& Subbands, cstr FileName);
void EncodeZfp(const f64* Data, v3i Dims, v3i TileDims, int Bits, f64 Tolerance,
               const dynamic_array<extent>& Subbands, bitstream* Bs);
void DecodeZfp(f64* Data, v3i Dims, v3i TileDims, int Bits, f64 Tolerance,
               const dynamic_array<extent>& Subbands, bitstream* Bs);

/* For reading a file into memory */
struct book_keeper {
  /* First index is by subband, second index is by tiles within each subband */
  typed_buffer<typed_buffer<linked_list<buffer>>> Chunks;
};

struct file_format {
  inline static int TileDim = 32; // dimensions of a tile are fixed at 32x32x32
  cstr FileName = nullptr;
  f64 Tolerance = 0;
  int Precision = 0; /* [0, 63] for double */
  int NumLevels = 0;
  volume Volume;
  bool DoWaveletTransform = false;
  bool DoExtrapolation = false;
};

/* API to use the file format */
void SetFileName(file_format* FileFormat, cstr FileName);
void SetTolerance(file_format* FileFormat, f64 Tolerance);
/* Precision = 63 means that the values in a zfp block are quantized to 63-bit integers,
and that 64 bit planes are encoded */
void SetPrecision(file_format* FileFormat, int Precision);
/* NLevels = 3 means performing the wavelet transform 3 times in each dimension */
void SetNumLevels(file_format* FileFormat, int NumLevels);
/* Pointer to the data, type and and size of the data */
void SetVolume(file_format* FileFormat, byte* Data, v3i Dims, data_type Type);
void SetWaveletTransform(file_format* FileFormat, bool DoWaveletTransform);
void SetExtrapolation(file_format* FileFormat, bool DoExtrapolation);
/* TODO: return an error code */
void Encode(const file_format& FileFormat);

} // namespace mg

#include "mg_zfp.inl"
