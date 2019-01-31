#include <stdio.h>
#include "mg_array.h"
#include "mg_bitstream.h"
#include "mg_common_types.h"
#include "mg_io.h"
#include "mg_logger.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_signal_processing.h"
#include "mg_timer.h"
#include "mg_types.h"
#include "mg_volume.h"
#include "mg_wavelet.h"
#include "mg_zfp.h"

namespace mg {

void EncodeBlock(const u64* Block, int Bitplane, int& N, bitstream* Bs) {
  /* extract bit plane Bitplane to X */
  u64 X = 0;
  for (int I = 0; I < 64; ++I)
    X += u64((Block[I] >> Bitplane) & 1u) << I;
  /* code the last N bits of bit plane b */
  WriteLong(Bs, X, N);
  X >>= N;
  for (; N < 64 && Write(Bs, !!X); X >>= 1, ++N)
    for (; N < 64 - 1 && !Write(Bs, X & 1u); X >>= 1, ++N) ;
}

void DecodeBlock(u64* Block, int Bitplane, int& N, bitstream* Bs) {
  /* decode first N bits of bit plane #Bitplane */
  u64 X = ReadLong(Bs, N);
  /* unary run-length decode remainder of bit plane */
  for (; N < 64 && Read(Bs); X += u64(1) << N++)
    for (; N < 64 - 1 && !Read(Bs); ++N) ;
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
}

/* TODO: handle multiple data types */
void EncodeData(const volume& Vol, v3i TileDims, int Bits, f64 Tolerance,
                const dynamic_array<extent>& Subbands, cstr FileName)
{
  // TODO: error handling
  // TODO: create the bit stream(s) inside the function
  v3i Dims = Extract3Ints(Vol.Dims);
  v3i BlockDims(4, 4, 4); // zfp block size
  FILE* Fp = nullptr;
  /* Count the total number of tiles in the whole volume */
  i64 NumTilesTotal = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandDims = Extract3Ints(Subbands[S].Dims);
    v3i NumTiles = (SubbandDims + TileDims - 1) / TileDims;
    NumTilesTotal += Prod<i64>(NumTiles);
  }
  mg_HeapArrayZero(TileHeaders, u64, NumTilesTotal + 1);
  const f64* Data = (const f64*)Vol.Buffer.Data;
  int ToleranceExp = Exponent(Tolerance);
  /* Loop through the subbands */
  i64 NumTilesSoFar = 0;
  // TODO: run the loop in parallel?
  // TODO: loop through the tiles in morton order
  int NumChunksMax = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = Extract3Ints(Subbands[S].Pos);
    v3i SubbandDims = Extract3Ints(Subbands[S].Dims);
    v3i NumTilesInSubband = (SubbandDims + TileDims - 1) / TileDims;
    /* Loop through the tiles */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      v3i RealTileDims(Min(SubbandPos.X + SubbandDims.X - TX, TileDims.X),
                       Min(SubbandPos.Y + SubbandDims.Y - TY, TileDims.Y),
                       Min(SubbandPos.Z + SubbandDims.Z - TZ, TileDims.Z));
      v3i NumBlocksInTile = ((RealTileDims + BlockDims) - 1) / BlockDims;
      i64 TileId = NumTilesSoFar +
        XyzToI(NumTilesInSubband, (v3i(TX, TY, TZ) - SubbandPos) / TileDims);
      // TODO: use the freelist allocator
      // TODO: use aligned memory allocation
      // TODO: try reusing the memory buffer
      mg_HeapArray(FloatTile, f64, Prod<i64>(TileDims));
      mg_HeapArray(IntTile, i64, Prod<i64>(TileDims));
      mg_HeapArray(UIntTile, u64, Prod<i64>(TileDims));
      mg_HeapArrayZero(Ns, i8, Prod<i64>(NumBlocksInTile));
      mg_HeapArray(EMaxes, i16, Prod<i64>(NumBlocksInTile));
      int ChunkId = 0;
      bitstream Bs;
      buffer CompressedChunkBuf;
      AllocateBuffer(&CompressedChunkBuf, 1000 * 1000);
      InitWrite(&Bs, CompressedChunkBuf);
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        /* loop through zfp blocks */
        for (int BZ = 0; BZ < RealTileDims.Z; BZ += BlockDims.Z) {
        for (int BY = 0; BY < RealTileDims.Y; BY += BlockDims.Y) {
        for (int BX = 0; BX < RealTileDims.X; BX += BlockDims.X) {
          i64 BlockId = XyzToI(NumBlocksInTile, v3i(BX, BY, BZ) / BlockDims);
          i64 K = XyzToI(NumBlocksInTile, v3i(BX, BY, BZ) / BlockDims) * Prod<i32>(BlockDims);
          if (Bitplane == Bits) {
            /* loop through the samples in each block */
            v3i RealBlockDims(Min(RealTileDims.X - BX, BlockDims.X),
                              Min(RealTileDims.Y - BY, BlockDims.Y),
                              Min(RealTileDims.Z - BZ, BlockDims.Z));
            for (int Z = 0; Z < RealBlockDims.Z; ++Z) {
            for (int Y = 0; Y < RealBlockDims.Y; ++Y) {
            for (int X = 0; X < RealBlockDims.X; ++X) {
              i64 I = XyzToI(Dims, v3i(TX + BX + X, TY + BY + Y, TZ + BZ + Z));
              i64 J = K + XyzToI(BlockDims, v3i(X, Y, Z));
              FloatTile[J] = Data[I]; // copy data to the local tile buffer
            }}} /* end loop through the samples in each block */
            /* Pad the block if necessary */
            PadBlock(&FloatTile[K], RealBlockDims.X, 1);
            PadBlock(&FloatTile[K], RealBlockDims.Y, 4);
            PadBlock(&FloatTile[K], RealBlockDims.Z, 16);
            int EMax = Quantize((byte*)&FloatTile[K], Prod<i32>(BlockDims), Bits - 1,
                                (byte*)&IntTile[K], data_type::float64);
            EMaxes[BlockId] = (i16)EMax;
            if (0 <= EMaxes[BlockId] - ToleranceExp + 1) {
              Write(&Bs, 1);
              // TODO: for now we don't care if the exponent is 2047 which represents Inf or NaN
              Write(&Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
            } else {
              Write(&Bs, 0);
            }
            ForwardBlockTransform(&IntTile[K]);
            ForwardShuffle(&IntTile[K], &UIntTile[K]);
          } // end of the first iteration
          /* encode */
          if (Bits - Bitplane <= EMaxes[BlockId] - ToleranceExp + 1) {
            int N = Ns[BlockId];
            EncodeBlock(&UIntTile[K], Bitplane, N, &Bs);
            Ns[BlockId] = (i8)N;
          }
          size_t Bytes = Size(&Bs);
          if (BlockId + 1 == Prod<i32>(NumBlocksInTile)) { // last block in the tile
            if (Bytes >= 4096 || (Bytes > 0 && Bitplane == 0)) { // dump the chunk to disk
              char FileNameBuf[256];
              snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", FileName, ChunkId++);
              NumChunksMax = Max(ChunkId, NumChunksMax);
              if (!(Fp = fopen(FileNameBuf, "r+b"))) { // if the file is not present, create it
                Fp = fopen(FileNameBuf, "wb");
                fwrite(TileHeaders.Data, sizeof(u64), NumTilesTotal + 1, Fp);
              } else { // file exists, go to the end
                mg_FSeek(Fp, 0, SEEK_END);
              }
              Flush(&Bs);
              u64 Where[2] = { 0 };
              Where[0] = mg_FTell(Fp);
              mg_Assert(Where[0] > 0);
              mg_Assert(Bytes > 0);
              fwrite(Bs.Stream.Data, Bytes, 1, Fp);
              Where[1] = mg_FTell(Fp);
              mg_Assert(Where[1] > Where[0]);
              mg_FSeek(Fp, sizeof(u64) * TileId, SEEK_SET);
              fwrite(Where, sizeof(Where), 1, Fp);
              fclose(Fp);
              InitWrite(&Bs, Bs.Stream);
            }  // end dump chunk to disk
          } // end last block of the tile
        }}} /* end loop through the zfp blocks */
      } // end loop through the bit planes
      DeallocateBuffer(&CompressedChunkBuf);
    }}} /* end loop through the tiles */
    NumTilesSoFar += Prod<i64>(NumTilesInSubband);
  } /* end loop through the subbands */
}

void DecodeData(volume* Vol, v3i TileDims, int Bits, f64 Tolerance,
                const dynamic_array<extent>& Subbands, cstr FileName)
{
  // TODO: error handling
  v3i Dims = Extract3Ints(Vol->Dims);
  v3i BlockDims(4, 4, 4); // zfp block size
  FILE* Fp = nullptr;
  f64* Data = (f64*)Vol->Buffer.Data;
  int ToleranceExp = Exponent(Tolerance);
  /* Loop through the subbands */
  i64 NumTilesSoFar = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = Extract3Ints(Subbands[S].Pos);
    v3i SubbandDims = Extract3Ints(Subbands[S].Dims);
    v3i NumTilesInSubband = (SubbandDims + TileDims - 1) / TileDims;
    /* Loop through the tiles */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      v3i RealTileDims(Min(SubbandPos.X + SubbandDims.X - TX, TileDims.X),
                       Min(SubbandPos.Y + SubbandDims.Y - TY, TileDims.Y),
                       Min(SubbandPos.Z + SubbandDims.Z - TZ, TileDims.Z));
      v3i NumBlocksInTile = ((RealTileDims + BlockDims) - 1) / BlockDims;
      i64 TileId = NumTilesSoFar +
        XyzToI(NumTilesInSubband, (v3i(TX, TY, TZ) - SubbandPos) / TileDims);
      mg_HeapArray(FloatTile, f64, Prod<i64>(TileDims));
      mg_HeapArray(IntTile, i64, Prod<i64>(TileDims));
      mg_HeapArrayZero(UIntTile, u64, Prod<i64>(TileDims));
      mg_HeapArrayZero(Ns, i8, Prod<i64>(NumBlocksInTile));
      mg_HeapArray(EMaxes, i16, Prod<i64>(NumBlocksInTile));
      int ChunkId = 0;
      bool ContinueDecoding = true;
      /* loop through the bit planes */
      bitstream Bs;
      buffer ChunkBuf;
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        if (Bitplane == Bits || Size(&Bs) >= 4096) {
          if (ChunkBuf.Bytes > 0)
            DeallocateBuffer(&ChunkBuf);
          char FileNameBuf[256];
          snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", FileName, ChunkId++);
          if ((Fp = fopen(FileNameBuf, "rb"))) {
            u64 Where[2] = { 0 };
            mg_FSeek(Fp, sizeof(u64) * TileId, SEEK_SET);
            fread(Where, sizeof(u64), 2, Fp); // read the pointers to the current and next chunks
            mg_Assert(Where[1] >= Where[0]);
            if (Where[1] > Where[0]) { // chunk exists
              AllocateBuffer(&ChunkBuf, Where[1] - Where[0]);
              mg_FSeek(Fp, Where[0], SEEK_SET);
              fread(ChunkBuf.Data, ChunkBuf.Bytes, 1, Fp); // read the chunk data
              InitRead(&Bs, ChunkBuf);
              fclose(Fp);
              InitRead(&Bs, ChunkBuf);
            }
            fclose(Fp);
          } else { // file does not exist
            ContinueDecoding = false;
          }
        }
        /* loop through zfp blocks in the tile */
        for (int BZ = 0; BZ < RealTileDims.Z; BZ += BlockDims.Z) {
        for (int BY = 0; BY < RealTileDims.Y; BY += BlockDims.Y) {
        for (int BX = 0; BX < RealTileDims.X; BX += BlockDims.X) {
          i64 BlockId = XyzToI(NumBlocksInTile, v3i(BX, BY, BZ) / BlockDims);
          i64 K = XyzToI(NumBlocksInTile, v3i(BX, BY, BZ) / BlockDims) * Prod<i32>(BlockDims);
          if (ContinueDecoding && Bitplane == Bits) { // only read emax in the first bitplane iteration
            mg_Assert(Bs.Stream.Bytes > 0);
            if (Read(&Bs)) // significant 
              EMaxes[BlockId] = Read(&Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
            else 
              EMaxes[BlockId] = ToleranceExp - 2;
          }
          /* decode block */
          if (ContinueDecoding && Bits - Bitplane <= EMaxes[BlockId] - ToleranceExp + 1) {
            mg_Assert(Bs.Stream.Bytes > 0);
            int N = Ns[BlockId];
            DecodeBlock(&UIntTile[K], Bitplane, N, &Bs);
            Ns[BlockId] = N;
          }
          /* last bit plane, copy the samples back */
          if (Bitplane == 0) {
            v3i RealBlockDims(Min(RealTileDims.X - BX, BlockDims.X),
                              Min(RealTileDims.Y - BY, BlockDims.Y),
                              Min(RealTileDims.Z - BZ, BlockDims.Z));
            InverseShuffle(&UIntTile[K], &IntTile[K]);
            InverseBlockTransform(&IntTile[K]);
            Dequantize((byte*)&IntTile[K], Prod<i32>(BlockDims), EMaxes[BlockId], Bits - 1,
                       (byte*)&FloatTile[K], data_type::float64);
            /* loop through the samples in each block */
            for (int Z = 0; Z < RealBlockDims.Z; ++Z) { /* loop through each block */
            for (int Y = 0; Y < RealBlockDims.Y; ++Y) {
            for (int X = 0; X < RealBlockDims.X; ++X) {
              i64 I = XyzToI(Dims, v3i(TX + BX + X, TY + BY + Y, TZ + BZ + Z));
              i64 J = K + XyzToI(BlockDims, v3i(X, Y, Z));
              Data[I] = FloatTile[J]; // copy data to the local tile buffer
            }}}
          } // end last bit plane
        }}} /* end loop through the zfp blocks */
      } // end loop through the bit planes
    }}} /* end loop through the tiles */
    NumTilesSoFar += Prod<i64>(NumTilesInSubband);
  } /* end loop through the subbands */
}

void SetFileName(file_format* FileFormat, cstr FileName) {
  FileFormat->FileName = FileName;
}
void SetTolerance(file_format* FileFormat, f64 Tolerance) {
  FileFormat->Tolerance = Tolerance;
}
void SetPrecision(file_format* FileFormat, int Precision) {
  FileFormat->Precision = Precision;
}
void SetNumLevels(file_format* FileFormat, int NumLevels) {
  FileFormat->NumLevels = NumLevels;
}
void SetVolume(file_format* FileFormat, byte* Data, v3i Dims, data_type Type) {
  FileFormat->Volume.Buffer.Data = Data;
  FileFormat->Volume.Buffer.Bytes = SizeOf(Type) * Prod<i64>(Dims);
  FileFormat->Volume.Dims = Stuff3Ints(Dims);
}
void SetWaveletTransform(file_format* FileFormat, bool DoWaveletTransform) {
  FileFormat->DoWaveletTransform = DoWaveletTransform;
}
void SetExtrapolation(file_format* FileFormat, bool DoExtrapolation) {
  FileFormat->DoExtrapolation = DoExtrapolation;
}
void Finalize(file_format* FileFormat) {
  // TODO: add more checking
  v3i Dims = Extract3Ints(FileFormat->Volume.Dims);
  int NDims = (Dims.X > 1) + (Dims.Y > 1) + (Dims.Z > 1);
  BuildSubbands(NDims, Dims, FileFormat->NumLevels, &FileFormat->Subbands);
  // TODO: initialize FileFormat->Chunks
}
void Encode(file_format* FileFormat) {
  if (FileFormat->DoExtrapolation) {
    // TODO
  }
  if (FileFormat->DoWaveletTransform)
    Cdf53Forward(&(FileFormat->Volume), FileFormat->NumLevels);
  EncodeData(FileFormat->Volume, v3i(FileFormat->TileDim), FileFormat->Precision,
             FileFormat->Tolerance, FileFormat->Subbands, FileFormat->FileName);
}

v3i GetNextChunk(file_format* FileFormat, v3i Level, v3i Tile, byte* Output) {
  int Sb = LevelToSubband(Level);
  // find out how many chunks the tile already has
  //int NumChunks = (int)Size(FileFormat->Chunks[Sb][]);
  // read the next chunk from disk and add to the end of the linked list
  // decode the whole tile from the beginning
}

} // namespace mg
