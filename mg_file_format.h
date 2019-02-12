#pragma once

#include "mg_array.h"
#include "mg_linked_list.h"
#include "mg_volume.h"
#include "mg_types.h"

namespace mg {

struct file_format {
  /* First index is by subband, second index is by tiles within each subband */
  volume Volume;
  typed_buffer<typed_buffer<linked_list<buffer>>> Chunks;
  dynamic_array<extent> Subbands;
  typed_buffer<u64> TileHeaders;
  v3i TileDims = v3i(32);
  cstr FileName = nullptr;
  f64 Tolerance = 0;
  int Precision = 63; /* [0, 63] for double */
  int NumLevels = 0;
  int ChunkBytes = 4096;
  bool DoExtrapolation = false;
  //TODO: add a flag to signify the file_format has been "finalized"
};

/* API to use the file format */
/* Must call */
void SetFileName(file_format* FileData, cstr FileName);
void SetVolume(file_format* FileData, byte* Data, v3i Dims, data_type Type);
/* TODO: return an error code */
void Encode(file_format* FileData);
void CleanUp(file_format* FileData);
/* Optional */
void SetTileDims(file_format* FileData, v3i TileDims); 
void SetTolerance(file_format* FileData, f64 Tolerance);
/* Precision = 63 means that the values in a zfp block are quantized to 63-bit integers,
and that 64 bit planes are encoded */
void SetPrecision(file_format* FileData, int Precision);
/* NLevels = 3 means performing the wavelet transform 3 times in each dimension */
void SetChunkBytes(file_format* FileData, int ChunkBytes);
/* Pointer to the data, type and and size of the data */
void SetWaveletTransform(file_format* FileData, int NumLevels);
void SetExtrapolation(file_format* FileData, bool DoExtrapolation);
/* Output should be an array large enough to hold a tile. Return the actual dimensions of
the tile. The coarsest subband is at level (0, 0, 0). Assuming the wavelte transform is
done in X, Y, then Z, the order of subbands is:
(0, 0, 0), (0, 0, 1), (0, 1, 0), (1, 0, 0), (1, 1, 1), ... */
v3i GetNextChunk(file_format* FileData, v3i Level, v3i Tile, byte* Output);

} // namespace mg

