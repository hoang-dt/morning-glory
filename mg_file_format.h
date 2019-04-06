#pragma once

#include "mg_array.h"
#include "mg_error.h"
#include "mg_linked_list.h"
#include "mg_volume.h"
#include "mg_types.h"

mg_Enum(ff_err_code, int, mg_CommonErrs,
  InvalidTileDims, InvalidChunkSize, ChunkReadFailed)

namespace mg {

#if defined(mg_CollectStats)
struct chunk_stats {
  int Where;
  u64 FirstEightBytes;
  int ActualSize;
  dynamic_array<int> Sizes;
};
struct tile_stats {
  int LocalId;
  dynamic_array<i16> EMaxes;
  dynamic_array<chunk_stats> CkStats;
};
struct subband_stats {
  int Sb;
  v3i NumTiles3;
  dynamic_array<tile_stats> TlStats;
};
struct file_stats {
  dynamic_array<subband_stats> SbStats;
};
inline file_stats FStats;
void Log(cstr FileName);
#endif

using ff_err = error<ff_err_code>;

// TODO: save some of these parameters to the file itself
struct file_format {
  int Major = 0;
  int Minor = 1;
  enum class mode : bool { Write, Read };
  /* First index is by subband, second index is by tiles within each subband */
  volume Volume;
  typed_buffer<typed_buffer<list<buffer>>> Chunks;
  dynamic_array<extent> Subbands;
  typed_buffer<u64> TileHeaders;
  v3i TileDims = v3i(32, 32, 32);
  cstr FileName = nullptr;
  f64 Tolerance = 0;
  int Prec = 64; /* [0, 64] for double, [0, 32] for float */
  int NLevels = 0;
  int ChunkBytes = 4096;
  bool DoExtrapolation = false;
  mode Mode = mode::Write;
  int MetaBytes = 0;
  char Meta[512] = "";
};

// TODO: add a "ReadNextChunk" public API of some sort (lowest-level API for reading)
/* API to use the file format */
/* Must call */
void SetFileName(file_format* Ff, cstr FileName);
void SetVolume(file_format* Ff, byte* Data, v3i Dims, data_type Type);
void SetTileDims(file_format* Ff, v3i TileDims);
void SetTolerance(file_format* Ff, f64 Tolerance);
/* Precision = 64 means that the values in a zfp block are quantized to 64-bit
 * integers (63-bit two-complement and 64-bit negabinary) */
void SetPrecision(file_format* Ff, int Precision);
void SetChunkBytes(file_format* Ff, int ChunkBytes);
void SetWaveletTransform(file_format* Ff, int NLevels);
void SetExtrapolation(file_format* Ff, bool DoExtrapolation);
ff_err Encode(file_format* Ff);
ff_err Decode(file_format* Ff);
void CleanUp(file_format* Ff);
/* Output should be an array large enough to hold a tile. Return the actual
 * dimensions of the tile. The coarsest subband is at level (0, 0, 0). Assuming
 * the wavelet transform is done in X, Y, then Z, the order of subbands is:
 * (0, 0, 0), (0, 0, 1), (0, 1, 0), (1, 0, 0), (1, 1, 1), ... */
//v3i GetNextChunk(file_format* Ff, v3i Level, v3i Tile, byte* Output);

} // namespace mg
