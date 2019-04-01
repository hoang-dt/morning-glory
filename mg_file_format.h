#pragma once

#include "mg_array.h"
#include "mg_error.h"
#include "mg_linked_list.h"
#include "mg_volume.h"
#include "mg_types.h"

mg_Enum(file_format_err_code, int, mg_CommonErrs,
  InvalidTileDims, InvalidChunkSize, ChunkReadFailed)

namespace mg {

using file_format_err = error<file_format_err_code>;

struct file_format {
  enum class mode : bool { Write, Read };
  /* First index is by subband, second index is by tiles within each subband */
  volume Volume;
  typed_buffer<typed_buffer<linked_list<buffer>>> Chunks;
  dynamic_array<extent> Subbands;
  typed_buffer<u64> TileHeaders;
  v3i TileDims = v3i(32, 32, 32);
  cstr FileName = nullptr;
  f64 Tolerance = 0;
  int Prec = 64; /* [0, 63] for double */
  int NLevels = 0;
  int ChunkBytes = 4096;
  bool DoExtrapolation = false;
  mode Mode = mode::Write;
  //TODO: add a flag to signify the file_format has been "finalized"
};

/* API to use the file format */
/* Must call */
void SetFileName(file_format* Ff, cstr FileName);
void SetVolume(file_format* Ff, byte* Data, v3i Dims, data_type Type);
file_format_err Finalize(file_format* Ff, file_format::mode Mode);
file_format_err Encode(file_format* Ff);
void CleanUp(file_format* Ff);
/* Optional */
void SetTileDims(file_format* Ff, v3i TileDims);
void SetTolerance(file_format* Ff, f64 Tolerance);
/* Precision = 64 means that the values in a zfp block are quantized to 64-bit
 * integers (63-bit two-complement and 64-bit negabinary) */
void SetPrecision(file_format* Ff, int Precision);
/* NLevels = 3 means performing the wavelet transform 3 times in each dimension */
void SetChunkBytes(file_format* Ff, int ChunkBytes);
/* Pointer to the data, type and and size of the data */
void SetWaveletTransform(file_format* Ff, int NLevels);
void SetExtrapolation(file_format* Ff, bool DoExtrapolation);
/* Output should be an array large enough to hold a tile. Return the actual
 * dimensions of the tile. The coarsest subband is at level (0, 0, 0). Assuming
 * the wavelet transform is done in X, Y, then Z, the order of subbands is:
 * (0, 0, 0), (0, 0, 1), (0, 1, 0), (1, 0, 0), (1, 1, 1), ... */
v3i GetNextChunk(file_format* Ff, v3i Level, v3i Tile, byte* Output);

} // namespace mg
