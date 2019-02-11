#include "mg_bitstream.h"
#include "mg_file_format.h"
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
    v3i SubbandDims = Extract3Ints(FileData.Subbands[S].Dims);
    v3i NumTiles = (SubbandDims + FileData.TileDims - 1) / FileData.TileDims;
    NumTilesInPrevSubbands += Prod<i64>(NumTiles);
  }
  return NumTilesInPrevSubbands;
}

int CopyBlockForward(const file_format& FileData, tile_data* TileData, v3i Block) {
  v3i Dims(FileData.Volume.Dims);
  v3i RealBlockDims = Min(FileData.TileDims - Block, ZfpBlockDims);
  const f64* Data = (f64*)FileData.Volume.Buffer.Data;
  v3i Voxel;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, TileData->Tile + Block + Voxel);
    i64 J = XyzToI(ZfpBlockDims, Voxel);
    TileData->Floats[J] = Data[I];
  } mg_EndFor3 
  PadBlock(TileData->Floats.Data, RealBlockDims.X, 1);
  PadBlock(TileData->Floats.Data, RealBlockDims.Y, 4);
  PadBlock(TileData->Floats.Data, RealBlockDims.Z, 16);
  return Quantize((byte*)TileData->Floats.Data, Prod<int>(ZfpBlockDims), FileData.Precision - 1,
                  (byte*)TileData->Ints.Data, FileData.Volume.Type);
}

/* TODO: Output should be something else? */
void CopyBlockInverse(const file_format& FileData, tile_data* TileData, v3i Block, f64* Output) {
  i64 BlockId = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims);
  v3i RealBlockDims = Min(TileData->RealDims - Block, ZfpBlockDims);
  InverseShuffle(TileData->UInts.Data, TileData->Ints.Data);
  InverseBlockTransform(TileData->Ints.Data);
  Dequantize((byte*)TileData->Ints.Data, Prod<int>(ZfpBlockDims), TileData->EMaxes[BlockId],
             FileData.Precision - 1, (byte*)TileData->Floats.Data, FileData.Volume.Type);
  v3i V;
  mg_BeginFor3(V, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(TileData->RealDims, Block + V);
    i64 J = XyzToI(ZfpBlockDims, V);
    Output[I] = TileData->Floats[J];
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
int WriteBlock(const file_format& FileData, tile_data* TileData, v3i Block, int ChunkId, int Bitplane) {
  i64 BlockId = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims);
  i64 K = XyzToI(TileData->NumBlocks, Block / ZfpBlockDims) * Prod<int>(ZfpBlockDims);
  if (ChunkId == 0) {
    TileData->EMaxes[BlockId] = (i16)CopyBlockForward(FileData, TileData, Block);
    WriteEMax(TileData->EMaxes[BlockId], Exponent(FileData.Tolerance), &TileData->Bs);
    ForwardBlockTransform(&TileData->Ints[K]);
    ForwardShuffle(&TileData->Ints[K], &TileData->UInts[K]);
  }
  if (FileData.Precision - Bitplane <= 
      TileData->EMaxes[BlockId] - Exponent(FileData.Tolerance) + 1) 
  {
ENCODE_NEXT:
    bool FullyEncoded = 
      EncodeBlock(&TileData->UInts[K], Bitplane, FileData.ChunkBytes * 8, TileData->Ns[BlockId],
                  TileData->Ms[BlockId], TileData->InnerLoops[BlockId], &TileData->Bs);
    bool ChunkComplete = Size(TileData->Bs) >= FileData.ChunkBytes;
    bool LastChunk = 
      (FullyEncoded) && (Bitplane == 0) && (BlockId + 1 == Prod<int>(TileData->NumBlocks));
    if (ChunkComplete || LastChunk) {
      if (!LastChunk)
        mg_Assert(Size(TileData->Bs) == FileData.ChunkBytes);
      WriteChunk(FileData, TileData, ChunkId++);
      if (!FullyEncoded)
        goto ENCODE_NEXT;
    } else {
      mg_Assert(FullyEncoded);
    }
  }
  return ChunkId;
}

void WriteTile(const file_format& FileData, tile_data* TileData) {
  int ChunkId = 0;
  InitWrite(&TileData->Bs, TileData->Bs.Stream);
  for (int Bitplane = FileData.Precision; Bitplane >= 0; --Bitplane) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), TileData->RealDims, ZfpBlockDims) {
      int NewChunkId = WriteBlock(FileData, TileData, Block, ChunkId, Bitplane);
      ChunkId = NewChunkId;
    } mg_EndFor3
  }
}

  // TODO: use the freelist allocator
  // TODO: use aligned memory allocation
  // TODO: try reusing the memory buffer
void WriteSubband(const file_format& FileData, int Sb) {
  v3i SubbandPos = Extract3Ints(FileData.Subbands[Sb].Pos);
  v3i SubbandDims = Extract3Ints(FileData.Subbands[Sb].Dims);
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
    AllocateTypedBuffer(&TileData.Ns, Prod<int>(TileData.NumBlocks));
    AllocateTypedBuffer(&TileData.Ms, Prod<int>(TileData.NumBlocks));
    AllocateTypedBuffer(&TileData.InnerLoops, Prod<int>(TileData.NumBlocks));
    AllocateBuffer(&TileData.Bs.Stream, FileData.ChunkBytes);
    WriteTile(FileData, &TileData);
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
void SetNumLevels(file_format* FileData, int NumLevels) {
  FileData->NumLevels = NumLevels;
}
void SetVolume(file_format* FileData, byte* Data, v3i Dims, data_type Type) {
  FileData->Volume.Buffer.Data = Data;
  FileData->Volume.Buffer.Bytes = SizeOf(Type) * Prod<i64>(Dims);
  FileData->Volume.Dims = Stuff3Ints(Dims);
}
void SetWaveletTransform(file_format* FileData, bool DoWaveletTransform) {
  FileData->DoWaveletTransform = DoWaveletTransform;
}
void SetExtrapolation(file_format* FileData, bool DoExtrapolation) {
  FileData->DoExtrapolation = DoExtrapolation;
}
void Finalize(file_format* FileData) {
  // TODO: add more checking
  v3i Dims = Extract3Ints(FileData->Volume.Dims);
  int NDims = (Dims.X > 1) + (Dims.Y > 1) + (Dims.Z > 1);
  BuildSubbands(NDims, Dims, FileData->NumLevels, &FileData->Subbands);
  AllocateTypedBuffer(&FileData->Chunks, Size(FileData->Subbands));
  for (int S = 0; S < Size(FileData->Subbands); ++S) {
    v3i SubbandDims = Extract3Ints(FileData->Subbands[S].Dims);
    v3i NumTiles = (SubbandDims + FileData->TileDims - 1) / FileData->TileDims;
    AllocateTypedBuffer(&FileData->Chunks[S], Prod<i64>(NumTiles));
  }
  int NumTilesTotal = GetNumTilesInPrevSubbands(*FileData, Size(FileData->Subbands));
  AllocateTypedBuffer(&FileData->TileHeaders, NumTilesTotal);
}

void Encode(file_format* FileData) {
  if (FileData->DoExtrapolation) {
    // TODO
  }
  if (FileData->DoWaveletTransform)
    Cdf53Forward(&(FileData->Volume), FileData->NumLevels);
  for (int Sb = 0; Sb < Size(FileData->Subbands); ++Sb)
    WriteSubband(*FileData, Sb);
}

// TODO: the read may fail
linked_list_iterator<buffer> ReadChunk(FILE* Fp, file_format* FileData, int Subband, 
                                       int TileIdInSubband, buffer* ChunkBuf, bitstream* Bs)
{
  mg_Assert(ChunkBuf->Bytes == FileData->ChunkBytes);
  fread(ChunkBuf->Data, ChunkBuf->Bytes, 1, Fp);
  auto& LinkedList = FileData->Chunks[Subband][TileIdInSubband];
  auto It = PushBack(&LinkedList, *ChunkBuf);
  InitRead(Bs, *It);
  return It;
}

// TODO: error handling
FILE* ReadTileHeader(file_format* FileData, int TileIdGlobal, int ChunkId) {
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", FileData->FileName, ChunkId);
  FILE* Fp = fopen(FileNameBuf, "rb");
  if (Fp) {
    mg_FSeek(Fp, sizeof(u64) * TileIdGlobal, SEEK_SET);
    u64 Where;
    fread(&Where, sizeof(u64), 1, Fp);
    if (Where == 0)
      return nullptr;
  }
  return Fp;
}

v3i GetNextChunk(file_format* FileData, v3i Level, v3i Tile, byte* Output) {
  (void)Output;
  // TODO: error and parameter checking
  // TODO: error handling
  int Sb = LevelToSubband(Level);
  // assert(Sb < Size(FileData->Subbands)
  int NumTilesInPrevSubbands = GetNumTilesInPrevSubbands(*FileData, Sb);
  v3i SubbandDims = Extract3Ints(FileData->Subbands[Sb].Dims);
  v3i TileDims = FileData->TileDims;
  // TODO: assert(Tile < NumTiles)
  int TileIdInSubband = XyzToI((SubbandDims + TileDims - 1) / TileDims, Tile);
  int TileIdGlobal = NumTilesInPrevSubbands + TileIdInSubband;
  v3i SubbandPos = Extract3Ints(FileData->Subbands[Sb].Pos);
  v3i T = SubbandPos + Tile * TileDims;
  v3i RealTileDims = Min(SubbandPos + SubbandDims - T, TileDims);
  int NumChunks = (int)Size(FileData->Chunks[Sb][TileIdInSubband]);
  FILE* Fp = ReadTileHeader(FileData, TileIdGlobal, NumChunks);
  if (Fp) {
    int ToleranceExp = Exponent(FileData->Tolerance);
    v3i BlockDims(4, 4, 4); // TODO: move into FileData
    v3i NumBlocksInTile = ((RealTileDims + BlockDims) - 1) / BlockDims;
    mg_HeapArray(FloatTile, f64, Prod<int>(TileDims));
    mg_HeapArray(IntTile, i64, Prod<int>(TileDims));
    mg_HeapArrayZero(UIntTile, u64, Prod<int>(TileDims));
    mg_HeapArrayZero(Ns, i8, Prod<int>(NumBlocksInTile));
    mg_HeapArrayZero(Ms, i8, Prod<int>(NumBlocksInTile));
    mg_HeapArrayZero(InnerLoops, bool, Prod<int>(NumBlocksInTile)); // TODO: merge with the above array
    mg_HeapArray(EMaxes, i16, Prod<int>(NumBlocksInTile));
    mg_HeapArrayZero(Visited, bool, Prod<int>(NumBlocksInTile)); // TODO: merge with the above array
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
          if (Read(&Bs)) // significant 
            EMaxes[BlockId] = Read(&Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
          else
            EMaxes[BlockId] = ToleranceExp - 2;
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
for (int S = 0; S < Size(FileData->Subbands); ++S) {
    for (i64 T = 0; T < Size(FileData->Chunks[S]); ++T) 
      Deallocate(&FileData->Chunks[S][T]);
    DeallocateTypedBuffer(&FileData->Chunks[S]);
  }
  DeallocateTypedBuffer(&FileData->Chunks);
  DeallocateTypedBuffer(&FileData->TileHeaders);
}

}

