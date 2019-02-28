#include "mg_bitstream.h"
#include "mg_expected.h"
#include "mg_file_format.h"
#include "mg_scopeguard.h"
#include "mg_math.h"
#include "mg_signal_processing.h"
#include "mg_wavelet.h"
#include "mg_zfp.h"

namespace mg {

struct tile_data {
  bitstream Bs;
  v3i Tile;
  v3i RealDims;
  v3i NumBlocks;
  int Subband;
  int IdInSubband;
  int IdGlobal;
  typed_buffer<f64> Floats;
  typed_buffer<i64> Ints;
  typed_buffer<u64> UInts;
  typed_buffer<i16> EMaxes;
  typed_buffer<i8> Ns;
  typed_buffer<i8> Ms;
  typed_buffer<bool> InnerLoops;
};

int GetNumTilesInPrevSubbands(const file_format& FileData, int Subband) {
  int NumTilesInPrevSubbands = 0;
  for (int S = 0; S < Subband; ++S) {
    v3i SubbandDims = Extract3Ints(FileData.Subbands[S].DimsCompact);
    v3i NumTiles = (SubbandDims + FileData.TileDims - 1) / FileData.TileDims;
    NumTilesInPrevSubbands += Prod<i64>(NumTiles);
  }
  return NumTilesInPrevSubbands;
}

template <typename t>
void CopyBlockForward(const file_format& FileData, tile_data* TileData, v3i Block, int K) {
  v3i Dims = Extract3Ints(FileData.Volume.DimsCompact);
  v3i RealBlockDims = Min(FileData.TileDims - Block, ZfpBlockDims);
  const t* Data = (t*)FileData.Volume.Buffer.Data;
  v3i Voxel;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, TileData->Tile + Block + Voxel);
    i64 J = K + XyzToI(ZfpBlockDims, Voxel);
    TileData->Floats[J] = Data[I];
  } mg_EndFor3
  PadBlock(TileData->Floats.Data, RealBlockDims.X, 1);
  PadBlock(TileData->Floats.Data, RealBlockDims.Y, 4);
  PadBlock(TileData->Floats.Data, RealBlockDims.Z, 16);
}

template <typename t>
void CopyBlockInverse(file_format* FileData, tile_data* TileData, v3i Block, int K, v3i Pos) {
  v3i Dims = Extract3Ints(FileData->Volume.DimsCompact);
  v3i RealBlockDims = Min(TileData->RealDims - Block, ZfpBlockDims);
  v3i Voxel;
  t* Data = (t*)FileData->Volume.Buffer.Data;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, Pos + Block + Voxel);
    i64 J = K + XyzToI(ZfpBlockDims, Voxel);
    Data[I] = TileData->Floats[J];
  } mg_EndFor3
}

void WriteEMax(int EMax, int ToleranceExp, bitstream* Bs) {
  if (0 <= EMax - ToleranceExp + 1) {
    Write(Bs, 1);
    // TODO: for now we don't care if the exponent is 2047 which represents Inf or NaN
    Write(Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
  } else {
    Write(Bs, 0);
  }
}

bool ReadEMax(bitstream* Bs, int ToleranceExp, i16* EMax) {
  if (Read(Bs)) {// significant
    *EMax = (i16)Read(Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
    return true;
  }
  *EMax = (i16)(ToleranceExp - 2);
  return false;
}

// TODO: error handling
// TODO: minimize fopen calls
void WriteChunk(const file_format& FileData, tile_data* TileData, int ChunkId) {
  if (Size(TileData->Bs) == 0)
    return;
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", FileData.FileName, ChunkId);
  FILE* Fp = fopen(FileNameBuf, "r+b");
  if (!Fp) { // if the file is not present, create it
    Fp = fopen(FileNameBuf, "wb");
    fwrite(FileData.TileHeaders.Data, Bytes(FileData.TileHeaders), 1, Fp);
  } else { // file exists, go to the end
    mg_FSeek(Fp, 0, SEEK_END); // TODO: this prevents parallelization in file I/O
  }
  Flush(&TileData->Bs);
  u64 Where = mg_FTell(Fp);
  fwrite(TileData->Bs.Stream.Data, FileData.ChunkBytes, 1, Fp);
  mg_FSeek(Fp, sizeof(u64) * TileData->IdGlobal, SEEK_SET);
  fwrite(&Where, sizeof(Where), 1, Fp);
  fclose(Fp);
  InitWrite(&TileData->Bs, TileData->Bs.Stream);
}

// TODO: minimize file opening
// TODO: the tile size should depend on the precision at some level, to reduce internal fragmentation
template <typename t>
int WriteBlock(const file_format& FileData, tile_data* TileData, 
               v3i Block, int ChunkId, int Bitplane) {
  i64 BlockId = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims);
  i64 K = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims) * Prod<int>(ZfpBlockDims);

  /* Copy the block data into the tile's buffer */
  if (Bitplane == FileData.Precision) {
    CopyBlockForward<t>(FileData, TileData, Block, K);
    TileData->EMaxes[BlockId] = (i16)Quantize(
        (byte*)&TileData->Floats[K], Prod<int>(ZfpBlockDims), FileData.Precision - 1,
        (byte*)&TileData->Ints[K], FileData.Volume.Type);
    WriteEMax(TileData->EMaxes[BlockId], Exponent(FileData.Tolerance), &TileData->Bs);
    ForwardBlockTransform(&TileData->Ints[K]);
    ForwardShuffle(&TileData->Ints[K], &TileData->UInts[K]);
  }

  /* Encode and write chunks */
  bool DoEncode = FileData.Precision - Bitplane <=
                  TileData->EMaxes[BlockId] - Exponent(FileData.Tolerance) + 1;
  bool LastChunk = (Bitplane == 0) && (BlockId + 1 == Prod<int>(TileData->NumBlocks));
  bool FullyEncoded = true;
  do {
    if (DoEncode) {
      FullyEncoded = EncodeBlock(
        &TileData->UInts[K], Bitplane, FileData.ChunkBytes * 8, TileData->Ns[BlockId],
        TileData->Ms[BlockId], TileData->InnerLoops[BlockId], &TileData->Bs);
    }
    bool ChunkComplete = Size(TileData->Bs) >= FileData.ChunkBytes;
    if (ChunkComplete || LastChunk)
      WriteChunk(FileData, TileData, ChunkId++);
  } while (!FullyEncoded);

  return ChunkId;
}

template <typename t>
void WriteTile(const file_format& FileData, tile_data* TileData) {
  int ChunkId = 0;
  InitWrite(&TileData->Bs, TileData->Bs.Stream);
  for (int Bitplane = FileData.Precision; Bitplane >= 0; --Bitplane) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), TileData->RealDims, ZfpBlockDims) {
      ChunkId = WriteBlock<t>(FileData, TileData, Block, ChunkId, Bitplane);
    } mg_EndFor3
  }
}

  // TODO: use the freelist allocator
  // TODO: use aligned memory allocation
  // TODO: try reusing the memory buffer
template <typename t>
void WriteSubband(const file_format& FileData, int Sb) {
  v3i SubbandPos = Extract3Ints(FileData.Subbands[Sb].PosCompact);
  v3i SubbandDims = Extract3Ints(FileData.Subbands[Sb].DimsCompact);
  v3i Tile;
  mg_BeginFor3(Tile, SubbandPos, SubbandPos + SubbandDims, FileData.TileDims) {
    v3i NumTilesInSubband = (SubbandDims + FileData.TileDims - 1) / FileData.TileDims;
    tile_data TileData;
    TileData.Tile = Tile;
    TileData.RealDims = Min(SubbandPos + SubbandDims - Tile, FileData.TileDims);
    TileData.IdInSubband = XyzToI(NumTilesInSubband, (Tile - SubbandPos) / FileData.TileDims);
    TileData.IdGlobal = GetNumTilesInPrevSubbands(FileData, Sb) + TileData.IdInSubband;
    TileData.NumBlocks = ((TileData.RealDims + ZfpBlockDims) - 1) / ZfpBlockDims;
    AllocateTypedBuffer(&TileData.Floats, Prod<int>(FileData.TileDims));
    AllocateTypedBuffer(&TileData.Ints, Prod<int>(FileData.TileDims));
    AllocateTypedBuffer(&TileData.UInts, Prod<int>(FileData.TileDims));
    AllocateTypedBuffer(&TileData.EMaxes, Prod<int>(TileData.NumBlocks));
    AllocateTypedBufferZero(&TileData.Ns, Prod<int>(TileData.NumBlocks));
    AllocateTypedBufferZero(&TileData.Ms, Prod<int>(TileData.NumBlocks));
    AllocateTypedBufferZero(&TileData.InnerLoops, Prod<int>(TileData.NumBlocks));
    AllocateBuffer(&TileData.Bs.Stream, FileData.ChunkBytes + BufferSize(TileData.Bs));
    WriteTile<t>(FileData, &TileData);
    DeallocateTypedBuffer(&TileData.Floats);
    DeallocateTypedBuffer(&TileData.Ints);
    DeallocateTypedBuffer(&TileData.UInts);
    DeallocateTypedBuffer(&TileData.EMaxes);
    DeallocateTypedBuffer(&TileData.Ns);
    DeallocateTypedBuffer(&TileData.Ms);
    DeallocateTypedBuffer(&TileData.InnerLoops);
    DeallocateBuffer(&TileData.Bs.Stream);
  } mg_EndFor3
}

void SetTileDims(file_format* FileData, v3i TileDims) {
  FileData->TileDims = TileDims;
}

void SetFileName(file_format* FileData, cstr FileName) {
  FileData->FileName = FileName;
}
void SetTolerance(file_format* FileData, f64 Tolerance) {
  FileData->Tolerance = Tolerance;
}
void SetPrecision(file_format* FileData, int Precision) {
  FileData->Precision = Precision;
}
void SetVolume(file_format* FileData, byte* Data, v3i Dims, data_type Type) {
  FileData->Volume.Buffer.Data = Data;
  FileData->Volume.Buffer.Bytes = SizeOf(Type) * Prod<i64>(Dims);
  FileData->Volume.DimsCompact = Stuff3Ints(Dims);
  FileData->Volume.Type = Type;
}
void SetWaveletTransform(file_format* FileData, int NumLevels) {
  FileData->NumLevels = NumLevels;
}
void SetExtrapolation(file_format* FileData, bool DoExtrapolation) {
  FileData->DoExtrapolation = DoExtrapolation;
}

/* TODO: we need to make sure that the chunk size is large enough to store all emaxes in
 * a tile */
error Finalize(file_format* FileData, file_format::mode Mode) {
  // TODO: add more checking
  v3i Dims = Extract3Ints(FileData->Volume.DimsCompact);
  int NDims = (Dims.X > 1) + (Dims.Y > 1) + (Dims.Z > 1);
  BuildSubbands(NDims, Dims, FileData->NumLevels, &FileData->Subbands);
  int NumTilesTotal = GetNumTilesInPrevSubbands(*FileData, Size(FileData->Subbands));
  AllocateTypedBuffer(&FileData->TileHeaders, NumTilesTotal);
  if (Mode == file_format::mode::Read) {
    /* allocate memory for the linked list */
    AllocateTypedBuffer(&FileData->Chunks, Size(FileData->Subbands));
    for (int S = 0; S < Size(FileData->Subbands); ++S) {
      v3i SubbandDims = Extract3Ints(FileData->Subbands[S].DimsCompact);
      v3i NumTiles = (SubbandDims + FileData->TileDims - 1) / FileData->TileDims;
      AllocateTypedBuffer(&FileData->Chunks[S], Prod<i64>(NumTiles));
      for (int T = 0; T < Size(FileData->Chunks[S]); ++T)
        new (&FileData->Chunks[S][T]) linked_list<buffer>;
    }
  }
  return error(error_code::NoError);
}

// TODO: error checking
error Encode(file_format* FileData) {
  if (FileData->DoExtrapolation) {
    // TODO
  }
  if (FileData->NumLevels > 0)
    Cdf53Forward(&(FileData->Volume), FileData->NumLevels);
  for (int Sb = 0; Sb < Size(FileData->Subbands); ++Sb) {
    if (FileData->Volume.Type == data_type::float64) {
      WriteSubband<f64>(*FileData, Sb);
    } else if (FileData->Volume.Type == data_type::float32) {
      WriteSubband<f32>(*FileData, Sb);
    } else {
      mg_Abort("Type not supported");
    }
  }
  return error(error_code::NoError);
}

error ReadChunk(file_format* FileData, tile_data* TileData, int ChunkId, buffer* ChunkBuf) {
  mg_Assert(ChunkBuf->Bytes == FileData->ChunkBytes);
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", FileData->FileName, ChunkId);
  FILE* Fp = fopen(FileNameBuf, "rb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  if (Fp) {
    if (fread(ChunkBuf->Data, ChunkBuf->Bytes, 1, Fp) == 1) {
      auto& LinkedList = FileData->Chunks[TileData->Subband][TileData->IdGlobal];
      auto It = PushBack(&LinkedList, *ChunkBuf);
      InitRead(&TileData->Bs, *It);
    } else { // cannot read the chunk in the file
      return mg_Error(FileReadFailed); 
    }
  } else { // the file does not exist
    return mg_Error(FileOpenFailed);
  }
  return error(error_code::NoError);
}

expected<u64> ReadTileHeader(file_format* FileData, const tile_data& TileData, int ChunkId) {
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", FileData->FileName, ChunkId);
  FILE* Fp = fopen(FileNameBuf, "rb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  u64 Where = 0;
  if (Fp) {
    mg_FSeek(Fp, sizeof(u64) * TileData.IdGlobal, SEEK_SET);
    if (fread(&Where, sizeof(u64), 1, Fp) != 1) {
      return mg_Error(FileReadFailed);
    }
  } else {
    return mg_Error(FileOpenFailed);
  }
  return Where;
}

/* Challenge: one block can straddle two chunks, and also a block may be decoded partially */
template <typename t>
error ReadBlock(file_format* FileData, tile_data* TileData,
                v3i Block, linked_list_iterator<buffer> ChunkIt, int Bitplane, v3i Pos) {
  i64 BlockId = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims);

  /* Read a new chunk from disk */
  if (Bitplane == FileData->Precision) 
    ReadEMax(&TileData->Bs, Exponent(FileData->Tolerance), &TileData->EMaxes[BlockId]);
  i64 K = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims) * Prod<int>(ZfpBlockDims);

  bool DoDecode = FileData->Precision - Bitplane <= 
                  TileData->EMaxes[BlockId] - Exponent(FileData->Tolerance) + 1;
  bool FulllyDecoded = false;
  error Err = error(error_code::NoError);
  const auto & LinkedList = FileData->Chunks[TileData->Subband][TileData->IdInSubband];
  while (!FulllyDecoded && ChunkIt != ConstEnd(LinkedList)) {
    if (DoDecode) {
      FulllyDecoded = DecodeBlock(
        &TileData->UInts[K], Bitplane, FileData->ChunkBytes * 8, TileData->Ns[BlockId],
        TileData->Ms[BlockId], TileData->InnerLoops[BlockId], &TileData->Bs);
    }
    if (!FulllyDecoded && ChunkId + 1 < Size(LinkedList)) {
      InitRead(&TileData->Bs, LinkedList->);
    }
  }

  if (Bitplane == 0) {
    InverseShuffle(TileData->UInts.Data, TileData->Ints.Data);
    InverseBlockTransform(TileData->Ints.Data);
    Dequantize((byte*)TileData->Ints.Data, Prod<int>(ZfpBlockDims), TileData->EMaxes[BlockId],
               FileData->Precision - 1, (byte*)TileData->Floats.Data, FileData->Volume.Type);
    CopyBlockInverse<t>(FileData, TileData, Block, K, Pos);
  }
  return Err;
}

/* TODO: What is Pos? */
// TODO: rethink this (we are not reading all chunks but one at a time)
template <typename t>
void ReadTile(file_format* FileData, tile_data* TileData, v3i Pos) {
  int ChunkId = 0;
  InitRead(&TileData->Bs, TileData->Bs.Stream);
  for (int Bitplane = FileData->Precision; Bitplane >= 0; --Bitplane) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), TileData->RealDims, ZfpBlockDims) {
      ChunkId = ReadBlock<t>(FileData, TileData, Block, ChunkId, Bitplane, Pos);
    } mg_EndFor3
  }
}

template <typename t>
void ReadSubband(file_format* FileData, int Subband, v3i Pos) {
  v3i SubbandPos = Extract3Ints(FileData->Subbands[Subband].PosCompact);
  v3i SubbandDims = Extract3Ints(FileData->Subbands[Subband].DimsCompact);
  v3i Tile;
  mg_BeginFor3(Tile, SubbandPos, SubbandPos + SubbandDims, FileData.TileDims) {
    v3i NumTilesInSubband = (SubbandDims + FileData->TileDims - 1) / FileData->TileDims;
    tile_data TileData;
    TileData.Tile = Tile;
    TileData.Subband = Subband;
    TileData.RealDims = Min(SubbandPos + SubbandDims - Tile, FileData->TileDims);
    TileData.IdInSubband = XyzToI(NumTilesInSubband, (Tile - SubbandPos) / FileData->TileDims);
    TileData.IdGlobal = GetNumTilesInPrevSubbands(FileData, Subband) + TileData.IdInSubband;
    TileData.NumBlocks = ((TileData.RealDims + ZfpBlockDims) - 1) / ZfpBlockDims;
    AllocateTypedBuffer(&TileData.Floats, Prod<int>(FileData->TileDims));
    AllocateTypedBuffer(&TileData.Ints, Prod<int>(FileData->TileDims));
    AllocateTypedBuffer(&TileData.UInts, Prod<int>(FileData->TileDims));
    AllocateTypedBuffer(&TileData.EMaxes, Prod<int>(TileData.NumBlocks));
    AllocateTypedBufferZero(&TileData.Ns, Prod<int>(TileData.NumBlocks));
    AllocateTypedBufferZero(&TileData.Ms, Prod<int>(TileData.NumBlocks));
    AllocateTypedBufferZero(&TileData.InnerLoops, Prod<int>(TileData.NumBlocks));
    AllocateBuffer(&TileData.Bs.Stream, FileData->ChunkBytes + BufferSize(TileData.Bs));
    u64 Where = ReadTileHeader(FileData, TileData, Size(FileData->Chunks[Subband][TileData.IdInSubband]));
    ReadTile<t>(FileData, &TileData, Pos);
    DeallocateTypedBuffer(&TileData.Floats);
    DeallocateTypedBuffer(&TileData.Ints);
    DeallocateTypedBuffer(&TileData.UInts);
    DeallocateTypedBuffer(&TileData.EMaxes);
    DeallocateTypedBuffer(&TileData.Ns);
    DeallocateTypedBuffer(&TileData.Ms);
    DeallocateTypedBuffer(&TileData.InnerLoops);
    DeallocateBuffer(&TileData.Bs.Stream);
  } mg_EndFor3
}

error GetNextChunk(file_format* FileData, v3i Level, v3i Tile, v3i Pos) {
  // TODO: error and parameter checking
  // TODO: error handling
  int Sb = LevelToSubband(Level);
  // assert(Sb < Size(FileData->Subbands)
  int NumTilesInPrevSubbands = GetNumTilesInPrevSubbands(*FileData, Sb);
  v3i SubbandDims = Extract3Ints(FileData->Subbands[Sb].DimsCompact);
  v3i TileDims = FileData->TileDims;
  // TODO: assert(Tile < NumTiles)
  int TileIdInSubband = XyzToI((SubbandDims + TileDims - 1) / TileDims, Tile);
  int TileIdGlobal = NumTilesInPrevSubbands + TileIdInSubband;
  v3i SubbandPos = Extract3Ints(FileData->Subbands[Sb].PosCompact);
  v3i T = SubbandPos + Tile * TileDims;
  v3i RealTileDims = Min(SubbandPos + SubbandDims - T, TileDims);
  int NumChunks = (int)Size(FileData->Chunks[Sb][TileIdInSubband]);
  if (Fp) {
    v3i NumBlocksInTile = ((RealTileDims + ZfpBlockDims) - 1) / ZfpBlockDims;
    buffer ChunkBuf;
    AllocateBuffer(&ChunkBuf, FileData->ChunkBytes);
    bitstream Bs;
    // TODO: check error
    auto It = ReadChunk(Fp, FileData, Sb, TileIdInSubband, &ChunkBuf, &Bs);
    It = Begin(FileData->Chunks[Sb][TileIdInSubband]);
    bool ContinueDecoding = true;
    /* loop through the bit planes */
    for (int Bitplane = FileData->Precision; Bitplane >= 0; --Bitplane) {
      v3i Block;
      mg_BeginFor3(Block, v3i::Zero(), RealTileDims, BlockDims) {
        i64 BlockId = XyzToI(NumBlocksInTile, Block / BlockDims);
        if (BlockId == 0 && Size(Bs) >= FileData->ChunkBytes) {
MOVE_TO_NEXT_CHUNK:
          ++It; // move on to the next chunk
          if (It == End(FileData->Chunks[Sb][TileIdInSubband]))
            ContinueDecoding = false;
          else
            InitRead(&Bs, *It);
        }
        i64 K = XyzToI(NumBlocksInTile, Block / BlockDims) * Prod<i32>(BlockDims);
        if (ContinueDecoding && !Visited[BlockId]) {
          Visited[BlockId] = true;
        }
        if (ContinueDecoding) { // TODO: add tolerance support?
          if (!DecodeBlock(&UIntTile[K], Bitplane, FileData->ChunkBytes * 8, Ns[BlockId],
                           Ms[BlockId], InnerLoops[BlockId], &Bs))
            goto MOVE_TO_NEXT_CHUNK;
        }
        // TODO
        //if (Bitplane == 0)
          //CopyBlockInverse(*FileData, RealTileDims, Block, BlockDims, &UIntTile[K],
                           //EMaxes[BlockId], &IntTile[K], &FloatTile[K], (f64*)Output);
      } mg_EndFor3
    } // end bit plane loop
  } else { // file does not exist
    // TODO
    return v3i(0);
  }
  return RealTileDims;
}

void CleanUp(file_format* FileData) {
  if (FileData->Mode == file_format::mode::Read) {
    for (int S = 0; S < Size(FileData->Subbands); ++S) {
      for (int T = 0; T < Size(FileData->Chunks[S]); ++T)
        Deallocate(&FileData->Chunks[S][T]);
      DeallocateTypedBuffer(&FileData->Chunks[S]);
    }
    if (Size(FileData->Chunks) > 0)
      DeallocateTypedBuffer(&FileData->Chunks);
  }
  if (Size(FileData->TileHeaders) > 0)
    DeallocateTypedBuffer(&FileData->TileHeaders);
}

}

