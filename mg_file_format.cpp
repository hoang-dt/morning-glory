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
void CopyBlockForward(const file_format& Fd, tile_data* Td, v3i Block, int K) {
  v3i Dims = Extract3Ints(Fd.Volume.DimsCompact);
  v3i RealBlockDims = Min(Fd.TileDims - Block, ZfpBlockDims);
  const t* Data = (t*)Fd.Volume.Buffer.Data;
  v3i Voxel;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, Td->Tile + Block + Voxel);
    i64 J = K + XyzToI(ZfpBlockDims, Voxel);
    Td->Floats[J] = Data[I];
  } mg_EndFor3
  PadBlock(Td->Floats.Data, RealBlockDims.X, 1);
  PadBlock(Td->Floats.Data, RealBlockDims.Y, 4);
  PadBlock(Td->Floats.Data, RealBlockDims.Z, 16);
}

template <typename t>
void CopyBlockInverse(file_format* Fd, tile_data* Td, v3i Block, int K) {
  v3i Dims = Extract3Ints(Fd->Volume.DimsCompact);
  v3i RealBlockDims = Min(Td->RealDims - Block, ZfpBlockDims);
  v3i Voxel;
  t* Data = (t*)Fd->Volume.Buffer.Data;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, Td->Tile + Block + Voxel);
    i64 J = K + XyzToI(ZfpBlockDims, Voxel);
    Data[I] = Td->Floats[J];
  } mg_EndFor3
}

void WriteEMax(int EMax, int ToleranceExp, bitstream* Bs) {
  if (0 <= EMax - ToleranceExp + 1) {
    Write(Bs, 1);
    // TODO: for now we don't care if the exponent is 2047 which represents Inf
    // or NaN
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

// TODO: write the header for the last chunk so during reading, we know when to sto
// TODO: error handling
// TODO: minimize fopen calls
void WriteChunk(const file_format& Fd, tile_data* Td, int Ci) {
  if (Size(Td->Bs) == 0)
    return;
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", Fd.FileName, Ci);
  FILE* Fp = fopen(FileNameBuf, "r+b");
  if (!Fp) { // if the file is not present, create it
    Fp = fopen(FileNameBuf, "wb");
    fwrite(Fd.TileHeaders.Data, Bytes(Fd.TileHeaders), 1, Fp);
  } else { // file exists, go to the end
    mg_FSeek(Fp, 0, SEEK_END); // TODO: this prevents parallelization in file I/O
  }
  Flush(&Td->Bs);
  u64 Where = mg_FTell(Fp);
  fwrite(Td->Bs.Stream.Data, Fd.ChunkBytes, 1, Fp);
  mg_FSeek(Fp, sizeof(u64) * Td->IdGlobal, SEEK_SET);
  fwrite(&Where, sizeof(Where), 1, Fp);
  fclose(Fp);
  InitWrite(&Td->Bs, Td->Bs.Stream);
}

// TODO: minimize file opening
// TODO: the tile size should depend on the precision at some level, to reduce
// internal fragmentation
template <typename t>
void WriteTile(const file_format& Fd, tile_data* Td) {
  int Ci = 0;
  InitWrite(&Td->Bs, Td->Bs.Stream);
  for (int Bp = Fd.Precision; Bp >= 0; --Bp) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), Td->RealDims, ZfpBlockDims) {
      int Bi = XyzToI(Td->NumBlocks, Block / ZfpBlockDims);
      int K = XyzToI(Td->NumBlocks, Block / ZfpBlockDims) * Prod(ZfpBlockDims);
      /* Copy the block data into the tile's buffer */
      if (Bp == Fd.Precision) {
        CopyBlockForward<t>(Fd, Td, Block, K);
        Td->EMaxes[Bi] = (i16)Quantize((byte*)&Td->Floats[K], Prod(ZfpBlockDims),
                                       Fd.Precision - 1, (byte*)&Td->Ints[K],
                                       Fd.Volume.Type);
        WriteEMax(Td->EMaxes[Bi], Exponent(Fd.Tolerance), &Td->Bs);
        ForwardBlockTransform(&Td->Ints[K]);
        ForwardShuffle(&Td->Ints[K], &Td->UInts[K]);
      }
      /* Encode and write chunks */
      bool DoEncode = Fd.Precision - Bp <=
                      Td->EMaxes[Bi] - Exponent(Fd.Tolerance) + 1;
      bool LastChunk = (Bp == 0) && (Bi + 1 == Prod(Td->NumBlocks));
      bool FullyEncoded = true;
      do {
        if (DoEncode) {
          FullyEncoded = EncodeBlock(&Td->UInts[K], Bp, Fd.ChunkBytes * 8,
                                     Td->Ns[Bi], Td->Ms[Bi],
                                     Td->InnerLoops[Bi], &Td->Bs);
        }
        bool ChunkComplete = Size(Td->Bs) >= Fd.ChunkBytes;
        if (ChunkComplete || LastChunk)
          WriteChunk(Fd, Td, Ci++);
      } while (!FullyEncoded);
    } mg_EndFor3
  }
}

// TODO: use the freelist allocator
// TODO: use aligned memory allocation
// TODO: try reusing the memory buffer
template <typename t>
void WriteSubband(const file_format& Fd, int Sb) {
  v3i SbPos = Extract3Ints(Fd.Subbands[Sb].PosCompact);
  v3i SbDims = Extract3Ints(Fd.Subbands[Sb].DimsCompact);
  v3i Tile;
  mg_BeginFor3(Tile, SbPos, SbPos + SbDims, Fd.TileDims) {
    v3i NTilesInSb = (SbDims + Fd.TileDims - 1) / Fd.TileDims;
    tile_data Td;
    Td.Tile = Tile;
    Td.RealDims = Min(SbPos + SbDims - Tile, Fd.TileDims);
    Td.IdInSubband = XyzToI(NTilesInSb, (Tile - SbPos) / Fd.TileDims);
    Td.IdGlobal = GetNumTilesInPrevSubbands(Fd, Sb) + Td.IdInSubband;
    Td.NumBlocks = ((Td.RealDims + ZfpBlockDims) - 1) / ZfpBlockDims;
    AllocateTypedBuffer(&Td.Floats, Prod<int>(Fd.TileDims));
    AllocateTypedBuffer(&Td.Ints, Prod<int>(Fd.TileDims));
    AllocateTypedBuffer(&Td.UInts, Prod<int>(Fd.TileDims));
    AllocateTypedBuffer(&Td.EMaxes, Prod<int>(Td.NumBlocks));
    AllocateTypedBufferZero(&Td.Ns, Prod<int>(Td.NumBlocks));
    AllocateTypedBufferZero(&Td.Ms, Prod<int>(Td.NumBlocks));
    AllocateTypedBufferZero(&Td.InnerLoops, Prod<int>(Td.NumBlocks));
    AllocateBuffer(&Td.Bs.Stream, Fd.ChunkBytes + BufferSize(Td.Bs));
    WriteTile<t>(Fd, &Td);
    DeallocateTypedBuffer(&Td.Floats);
    DeallocateTypedBuffer(&Td.Ints);
    DeallocateTypedBuffer(&Td.UInts);
    DeallocateTypedBuffer(&Td.EMaxes);
    DeallocateTypedBuffer(&Td.Ns);
    DeallocateTypedBuffer(&Td.Ms);
    DeallocateTypedBuffer(&Td.InnerLoops);
    DeallocateBuffer(&Td.Bs.Stream);
  } mg_EndFor3
}

void SetTileDims(file_format* Fd, v3i TileDims) {
  Fd->TileDims = TileDims;
}

void SetFileName(file_format* Fd, cstr FileName) {
  Fd->FileName = FileName;
}
void SetTolerance(file_format* Fd, f64 Tolerance) {
  Fd->Tolerance = Tolerance;
}
void SetPrecision(file_format* Fd, int Precision) {
  Fd->Precision = Precision;
}
void SetVolume(file_format* Fd, byte* Data, v3i Dims, data_type Type) {
  Fd->Volume.Buffer.Data = Data;
  Fd->Volume.Buffer.Bytes = SizeOf(Type) * Prod<i64>(Dims);
  Fd->Volume.DimsCompact = Stuff3Ints(Dims);
  Fd->Volume.Type = Type;
}
void SetWaveletTransform(file_format* Fd, int NumLevels) {
  Fd->NumLevels = NumLevels;
}
void SetExtrapolation(file_format* Fd, bool DoExtrapolation) {
  Fd->DoExtrapolation = DoExtrapolation;
}

// TODO: change Dims to NSamples3
// TODO: change AllocateTypedBuffer to AllocTypedBuf

/* TODO: we need to make sure that the chunk size is large enough to store all
 *  emaxes in a tile */
file_format_err Finalize(file_format* Fd, file_format::mode Mode) {
  // TODO: add more checking
  v3i Dims = Extract3Ints(Fd->Volume.DimsCompact);
  BuildSubbands(Dims, Fd->NumLevels, &Fd->Subbands);
  // TODO: use i64 for NTilesTotal
  int NTilesTotal = GetNumTilesInPrevSubbands(*Fd, Size(Fd->Subbands));
  AllocateTypedBufferZero(&Fd->TileHeaders, NTilesTotal + 1);
  if (Mode == file_format::mode::Read) {
    /* allocate memory for the linked list */
    AllocateTypedBuffer(&Fd->Chunks, Size(Fd->Subbands));
    for (int Sb = 0; Sb < Size(Fd->Subbands); ++Sb) {
      v3i SbDims = Extract3Ints(Fd->Subbands[Sb].DimsCompact);
      v3i NTiles3 = (SbDims + Fd->TileDims - 1) / Fd->TileDims;
      AllocateTypedBuffer(&Fd->Chunks[Sb], Prod<i64>(NTiles3));
      for (int Ti = 0; Ti < Size(Fd->Chunks[Sb]); ++Ti)
        new (&Fd->Chunks[Sb][Ti]) linked_list<buffer>;
    }
  }
  return mg_Error(file_format_err_code::NoError);
}

// TODO: error checking
file_format_err Encode(file_format* Fd) {
  if (Fd->DoExtrapolation) {
    // TODO
  }
  if (Fd->NumLevels > 0)
    Cdf53Forward(&(Fd->Volume), Fd->NumLevels);
  for (int Sb = 0; Sb < Size(Fd->Subbands); ++Sb) {
    if (Fd->Volume.Type == data_type::float64) {
      WriteSubband<f64>(*Fd, Sb);
    } else if (Fd->Volume.Type == data_type::float32) {
      WriteSubband<f32>(*Fd, Sb);
    } else {
      mg_Abort("Type not supported");
    }
  }
  return mg_Error(file_format_err_code::NoError);
}

/* Read the next chunk from disk */
file_format_err ReadNextChunk(file_format* Fd, tile_data* Td, buffer* ChunkBuf)
{
  mg_Assert(ChunkBuf->Bytes == Fd->ChunkBytes);
  auto& ChunkList = Fd->Chunks[Td->Subband][Td->IdGlobal];
  int ChunkId = int(Size(ChunkList));
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", Fd->FileName, ChunkId);
  FILE* Fp = fopen(FileNameBuf, "rb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  /* Read the chunk header */
  u64 Where = 0;
  if (Fp) {
    mg_FSeek(Fp, sizeof(u64) * Td->IdGlobal, SEEK_SET);
    if (fread(&Where, sizeof(u64), 1, Fp) != 1)
      return mg_Error(file_format_err_code::FileReadFailed);
  } else {
    return mg_Error(file_format_err_code::FileOpenFailed);
  }
  /* Read the chunk data */
  if (Where > 0) {
    if (fread(ChunkBuf->Data, ChunkBuf->Bytes, 1, Fp) == 1) {
      auto ChunkIt = PushBack(&ChunkList, *ChunkBuf);
      InitRead(&Td->Bs, *ChunkIt);
    } else { // cannot read the chunk in the file
      return mg_Error(file_format_err_code::FileReadFailed);
    }
  } else { // the chunk does not exist
    return mg_Error(file_format_err_code::ChunkReadFailed);
  }
  return error(file_format_err_code::NoError);
}

template <typename t>
void DecompressTile(file_format* Fd, tile_data* Td) {
  InitRead(&Td->Bs, Td->Bs.Stream);
  const auto & ChunkList = Fd->Chunks[Td->Subband][Td->IdInSb];
  auto ChunkIt = ConstBegin(ChunkList);
  for (int Bp = Fd->Precision; Bp >= 0; --Bp) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), Td->RealDims, ZfpBlockDims) {
      int Bi = XyzToI(Td->NumBlocks, Block / ZfpBlockDims);
      bool DoDecode = false;
      if (Bp == Fd->Precision)
        DoDecode = ReadEMax(&Td->Bs, Exponent(Fd->Tolerance), &Td->EMaxes[Bi]);
      int K = XyzToI(Td->NumBlocks, Block / ZfpBlockDims) * Prod(ZfpBlockDims);
      bool DoDecode = Fd->Precision - Bp <=
                      Td->EMaxes[Bi] - Exponent(Fd->Tolerance) + 1;
      bool FullyDecoded = false;
      bool LastChunk = (*ChunkIt) == ConstEnd(ChunkList);
      bool ExhaustedBits = BitSize(Td->Bs) >= file_format::ChunkSize;
      while (DoDecode && !FullyDecoded && !LastChunk) {
        FullyDecoded = DecodeBlock(&Td->UInts[K], Bp, Fd->ChunkBytes * 8,
                                   Td->Ns[Bi], Td->Ms[Bi],
                                   Td->InnerLoops[Bi], &Td->Bs);
        ExhaustedBits = BitSize(Td->Bs) >= file_format::ChunkSize;
        if (ExhaustedBits) {
          ++(*ChunkIt);
          LastChunk = (*ChunkIt) == ConstEnd(ChunkList);
          if (!LastChunk)
            InitRead(&Td->Bs, **ChunkIt);
        } else {
          mg_Assert(FullyDecoded);
        }
      }

      if (Bp == 0) {
        InverseShuffle(Td->UInts.Data, Td->Ints.Data);
        InverseBlockTransform(Td->Ints.Data);
        Dequantize((byte*)Td->Ints.Data, Prod(ZfpBlockDims),
                   Td->EMaxes[Bi], Fd->Precision - 1,
                   (byte*)Td->Floats.Data, Fd->Volume.Type);
        CopyBlockInverse<t>(Fd, Td, Block, K);
      }
      if (LastChunk)
        goto END;
    } mg_EndFor3
  }
END:
}

template <typename t>
file_format_err ImproveTile(file_format* Fd, int Sb, v3i Tile) {
  mg_Assert(Sb >= 0 && Sb < Size(Fd->Subbands));
  v3i SbPos = Extract3Ints(Fd->Subbands[Sb].PosCompact);
  v3i SbDims = Extract3Ints(Fd->Subbands[Sb].DimsCompact);
  v3i NTilesInSb3 = (SbDims + Fd->TileDims - 1) / Fd->TileDims;
  mg_Assert(Tile < NTilesInSb3);
  tile_data Td;
  Td.Tile = Tile;
  Td.Subband = Sb;
  Td.RealDims = Min(SbPos + SbDims - Tile, Fd->TileDims);
  Td.IdInSb = XyzToI(NTilesInSb3, (Tile - SbPos) / Fd->TileDims);
  Td.IdGlobal = GetNumTilesInPrevSubbands(Fd, Sb) + Td.IdInSb;
  Td.NumBlocks = ((Td.RealDims + ZfpBlockDims) - 1) / ZfpBlockDims;

  // TODO: allocate only once for all the structures
  // TODO: reuse memory (and only deallocate at the end of everything)
  AllocateTypedBuffer(&Td.Floats, Prod(Fd->TileDims));
  AllocateTypedBuffer(&Td.Ints, Prod(Fd->TileDims));
  AllocateTypedBuffer(&Td.UInts, Prod(Fd->TileDims));
  AllocateTypedBuffer(&Td.EMaxes, Prod(Td.NumBlocks));
  AllocateTypedBufferZero(&Td.Ns, Prod(Td.NumBlocks));
  AllocateTypedBufferZero(&Td.Ms, Prod(Td.NumBlocks));
  AllocateTypedBufferZero(&Td.InnerLoops, Prod(Td.NumBlocks));
  AllocateBuffer(&Td.Bs.Stream, Fd->ChunkBytes + BufferSize(Td.Bs));
  buffer ChunkBuf;
  file_format_err Err = ReadNextChunk(Fd, Td, &ChunkBuf);
  ReadTile<t>(Fd, &TileData, Pos);
  // TODO: copy the data from tiledata to outside
  DeallocateTypedBuffer(&Td.Floats);
  DeallocateTypedBuffer(&Td.Ints);
  DeallocateTypedBuffer(&Td.UInts);
  DeallocateTypedBuffer(&Td.EMaxes);
  DeallocateTypedBuffer(&Td.Ns);
  DeallocateTypedBuffer(&Td.Ms);
  DeallocateTypedBuffer(&Td.InnerLoops);
  DeallocateBuffer(&Td.Bs.Stream);
}

void CleanUp(file_format* Fd) {
  if (Fd->Mode == file_format::mode::Read) {
    for (int S = 0; S < Size(Fd->Subbands); ++S) {
      for (int T = 0; T < Size(Fd->Chunks[S]); ++T)
        Deallocate(&Fd->Chunks[S][T]);
      DeallocateTypedBuffer(&Fd->Chunks[S]);
    }
    if (Size(Fd->Chunks) > 0)
      DeallocateTypedBuffer(&Fd->Chunks);
  }
  if (Size(Fd->TileHeaders) > 0)
    DeallocateTypedBuffer(&Fd->TileHeaders);
}

}

