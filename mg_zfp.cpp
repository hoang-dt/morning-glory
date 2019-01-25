#include <stdio.h>
#include "mg_array.h"
#include "mg_bitstream.h"
#include "mg_common_types.h"
#include "mg_io.h"
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
                const dynamic_array<extent>& Subbands, cstr FileName, bitstream* Bs) 
{
  // TODO: error handling
  v3i Dims = Extract3Ints(Vol.Dims);
  v3i BlockDims{ 4, 4, 4 }; // zfp block
  FILE* Fp = nullptr;
  /* Count the total number of tiles in the whole volume */
  i64 NumTilesTotal = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandDims = Extract3Ints(Subbands[S].Dims);
    v3i NumTiles = (SubbandDims + TileDims - 1) / TileDims;
    NumTilesTotal += Prod<i64>(NumTiles);
  }
  mg_HeapArrayZero(TileHeaders, u64, NumTilesTotal);
  // Reserve space for the tile pointers
  const f64* Data = (const f64*)Vol.Buffer.Data;
  int ToleranceExp = Exponent(Tolerance);
  /* Loop through the subbands */
  i64 NumTilesSoFar = 0;
  // TODO: run the loop in parallel?
  // TODO: loop through the tiles in morton order
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
      i64* IntTile = (i64*)FloatTile.Data;
      mg_HeapArray(UIntTile, u64, Prod<i64>(TileDims));
      mg_HeapArrayZero(Ns, i8, Prod<i64>(NumBlocksInTile));
      mg_HeapArray(EMaxes, i16, Prod<i64>(NumBlocksInTile));
      int ChunkId = 0;
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        /* loop through zfp blocks */
        for (int BZ = 0; BZ < RealTileDims.Z; BZ += BlockDims.Z) {
        for (int BY = 0; BY < RealTileDims.Y; BY += BlockDims.Y) {
        for (int BX = 0; BX < RealTileDims.X; BX += BlockDims.X) {
          i64 BlockId = XyzToI(NumBlocksInTile, v3i(BX, BY, BZ) / BlockDims);
          i64 K = XyzToI(NumBlocksInTile, v3i(BX, BY, BZ) / BlockDims) * Prod<i32>(BlockDims);
          if (Bitplane == Bits) { // only copy the samples on the first loop iteration
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
            int EMax = Quantize(&FloatTile[K], Prod<i32>(BlockDims), Bits - 1,
                                &IntTile[K], data_type::float64);
            EMaxes[BlockId] = (i16)EMax;
            if (0 <= EMaxes[BlockId] - ToleranceExp + 1) {
              Write(Bs, 1);
              // TODO: for now we don't care if the exponent is 2047 which represents Inf or NaN
              Write(Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
            } else {
              Write(Bs, 0);
            }
            ForwardBlockTransform(&IntTile[K]);
            ForwardShuffle(&IntTile[K], &UIntTile[K]);
          } // end of the first iteration
          /* encode */
          if (Bits - Bitplane <= EMaxes[BlockId] - ToleranceExp + 1) {
            int N = Ns[BlockId];
            EncodeBlock(&UIntTile[K], Bitplane, N, Bs);
            Ns[BlockId] = (i8)N;
          }
          size_t Bytes = Size(Bs);
          if (BlockId + 1 == Prod<i32>(NumBlocksInTile)) { // last block in the tile
            if (Bytes >= 4096 || (Bytes > 0 && Bitplane == 0)) { // dump the chunk to disk
              char FileNameBuf[256];
              snprintf(FileNameBuf, 256, "%s%d", FileName, ChunkId++);
              Fp = fopen(FileNameBuf, "r+b");
              if (!Fp) { // if the file is not present, create it
                Fp = fopen(FileNameBuf, "wb");
                fwrite(&TileHeaders.Data, sizeof(u64), NumTilesTotal, Fp);
              } else { // file exists, go to the end
                mg_FSeek(Fp, 0, SEEK_END);
              }
              Flush(Bs);
              u64 Where = mg_FTell(Fp);
              fwrite(Bs->Stream.Data, Bytes, 1, Fp);
              mg_FSeek(Fp, sizeof(u64) * TileId, SEEK_SET);
              fwrite(&Where, sizeof(Where), 1, Fp);
              fclose(Fp);
              InitWrite(Bs, Bs->Stream);
            } 
          } // end last block of the tile
        }}} /* end loop through the zfp blocks */
      } // end loop through the bit planes
    }}} /* end loop through the tiles */
    NumTilesSoFar += Prod<i64>(NumTilesInSubband);
  } /* end loop through the subbands */
}

void DecodeData(f64* Data, v3i Dims, v3i TileDims) {
  // TODO: use many different bit streams
  FILE* Fp = fopen("compressed.raw", "rb");
  v3i BlockDims{ 4, 4, 4 };
  v3i NumTiles = (Dims + TileDims - 1) / TileDims;
  // fseek(Fp, sizeof(void*) * Prod<i64>(NumTiles), SEEK_SET); // reserve space for the tile pointers
  mg_HeapArray(TilePointers, size_t, Prod<i64>(NumTiles));
  fread(TilePointers.Data, sizeof(size_t) * Prod<i64>(NumTiles), 1, Fp);
  v3i NumBlocks = ((TileDims + BlockDims) - 1) / BlockDims;
  for (int TZ = 0; TZ < Dims.Z; TZ += TileDims.Z) { /* loop through the tiles */
  for (int TY = 0; TY < Dims.Y; TY += TileDims.Y) {
  for (int TX = 0; TX < Dims.X; TX += TileDims.X) {
    i64 TileId = XyzToI(NumTiles, v3i{ TX, TY, TZ } / TileDims);
    // TODO: use the freelist allocator
    // TODO: use aligned memory allocation
    // TODO: try reusing the memory buffer
    mg_HeapArray(FloatTile, f64, Prod<i64>(TileDims));
    mg_HeapArray(IntTile, i64, Prod<i64>(TileDims));
    mg_HeapArray(UIntTile, u64, Prod<i64>(TileDims));
    memset(UIntTile.Data, 0, sizeof(u64) * Prod<i64>(TileDims));
    mg_HeapArray(Ns, int, Prod<i64>(NumBlocks));
    memset(Ns.Data, 0, sizeof(int) * Prod<i64>(NumBlocks));
    mg_HeapArray(EMaxes, int, Prod<i64>(NumBlocks));
    mg_FSeek(Fp, TilePointers[TileId], SEEK_SET);
    int NumChunks = 0;
    fread(&NumChunks, sizeof(NumChunks), 1, Fp);
    short ChunkBytes[64] = { 0 };
    fread(ChunkBytes, sizeof(ChunkBytes[0]) * NumChunks, 1, Fp);
    /* compute the total size of the chunks */
    size_t TotalChunkSize = 0;
    for (int I = 0; I < NumChunks; ++I) {
      TotalChunkSize += ChunkBytes[I];
    }
    mg_FSeek(Fp, TilePointers[TileId] - TotalChunkSize, SEEK_SET);
    dynamic_array<byte> Buf;
    Resize(&Buf, ChunkBytes[0]);
    // fprintf(stderr, "0 (%ld)", ftell(Fp));
    fread(Buf.Buffer.Data, ChunkBytes[0], 1, Fp);
    bitstream Bs;
    InitRead(&Bs, Buf.Buffer);
    int ChunkId = 0;
    for (int Bitplane = 63; Bitplane >= 0; --Bitplane) {
      for (int BZ = 0; BZ < TileDims.Z; BZ += BlockDims.Z) { /* loop through zfp blocks */
      for (int BY = 0; BY < TileDims.Y; BY += BlockDims.Y) {
      for (int BX = 0; BX < TileDims.X; BX += BlockDims.X) {
        i64 BlockId = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims);
        if (Bitplane == 63) {
          int EMax = Read(&Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
          EMaxes[BlockId] = EMax; // save EMax here
        }
        int& N = Ns[BlockId];
        i64 K = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims) * Prod<i32>(BlockDims);
        DecodeBlock(&UIntTile[K], Bitplane, N, &Bs);
        if (Bitplane == 0) { // copy data back in the last loop iteratoin
          InverseShuffle(&UIntTile[K], &IntTile[K]);
          InverseBlockTransform(&IntTile[K]);
          Dequantize(&IntTile[K], Prod<i32>(BlockDims), EMaxes[BlockId], 52,
                     &FloatTile[K], data_type::float64); // TODO: 64 bit planes?
          for (int Z = 0; Z < BlockDims.Z; ++Z) { /* loop through each block */
          for (int Y = 0; Y < BlockDims.Y; ++Y) {
          for (int X = 0; X < BlockDims.X; ++X) {
            i64 I = XyzToI(Dims, v3i{ TX + BX + X, TY + BY + Y, TZ + BZ + Z });
            i64 J = K + XyzToI(BlockDims, v3i{ X, Y, Z });
            Data[I] = FloatTile[J]; // copy data to the local tile buffer
          }}}
        }
        /* read the next chunk if necessary */
        if (BlockId + 1 == Prod<i32>(NumBlocks)) {
          if (Size(&Bs) >= 4096 || Bitplane == 0) {
            if (ChunkId < NumChunks - 1) {
              // fprintf(stderr, "%d ", ChunkId + 1);
              Resize(&Buf, ChunkBytes[++ChunkId]);
              // fprintf(stderr, "(%ld)", ftell(Fp));
              fread(Buf.Buffer.Data, ChunkBytes[ChunkId], 1, Fp);
              InitRead(&Bs, Buf.Buffer);
            }
          }
        }
      }}} /* end loop through each block */
    } // end loop through the bit planes
    // fprintf(stderr, "\n");
    // TODO: padding?
  }}} /* end loop through the tiles */
  fclose(Fp);
}

/* The "simple" version */
void EncodeZfp(const f64* Data, v3i Dims, v3i TileDims, int Bits, f64 Tolerance,
               const dynamic_array<extent>& Subbands, bitstream* Bs) {
  timer Timer;
  StartTimer(&Timer);
  // TODO: loop through the subbands
  // TODO: error handling
  v3i BlockDims{ 4, 4, 4 }; // zfp block
  v3i NumBlocks = ((TileDims + BlockDims) - 1) / BlockDims;
  // TODO: run the loop in parallel?
  int ToleranceExp = Exponent(Tolerance);
  /* loop through the subbands */
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = Extract3Ints(Subbands[S].Pos);
    v3i SubbandDims = Extract3Ints(Subbands[S].Dims);
    /* loop through the tiles */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
    // for (int TZ = 0; TZ < Dims.Z; TZ += TileDims.Z) {
    // for (int TY = 0; TY < Dims.Y; TY += TileDims.Y) {
    // for (int TX = 0; TX < Dims.X; TX += TileDims.X) {
      // TODO: use the freelist allocator
      // TODO: use aligned memory allocation
      // TODO: try reusing the memory buffer
      mg_HeapArray(FloatTile, f64, Prod<i64>(TileDims));
      mg_HeapArray(IntTile, i64, Prod<i64>(TileDims));
      mg_HeapArray(UIntTile, u64, Prod<i64>(TileDims));
      mg_HeapArray(Ns, int, Prod<i64>(NumBlocks));
      memset(Ns.Data, 0, sizeof(int) * Prod<i64>(NumBlocks));
      mg_HeapArray(EMaxes, int, Prod<i64>(NumBlocks));
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        /* loop through zfp blocks */
        for (int BZ = 0; BZ < TileDims.Z; BZ += BlockDims.Z) {
        for (int BY = 0; BY < TileDims.Y; BY += BlockDims.Y) {
        for (int BX = 0; BX < TileDims.X; BX += BlockDims.X) {
          i64 BI = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims);
          // TODO: handle partial blocks
          i64 K = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims) * Prod<i32>(BlockDims);
          if (Bitplane == Bits) { // only do the following on the first loop iteration
            /* loop through the samples in each block */
            for (int Z = 0; Z < BlockDims.Z; ++Z) {
            for (int Y = 0; Y < BlockDims.Y; ++Y) {
            for (int X = 0; X < BlockDims.X; ++X) {
                i64 I = XyzToI(Dims, v3i{ TX + BX + X, TY + BY + Y, TZ + BZ + Z });
                i64 J = K + XyzToI(BlockDims, v3i{ X, Y, Z });
                FloatTile[J] = Data[I]; // copy data to the local tile buffer
            }}} /* end loop through the samples in each block */
            // TODO: figure out the number of bit planes to encode (range expansion issues)
            // TODO: add support for fixed accuracy coding
            int EMax = Quantize(&FloatTile[K], Prod<i32>(BlockDims), Bits - 1,
                                &IntTile[K], data_type::float64);
            EMaxes[BI] = EMax;
            // TODO: for now we don't care if the exponent is 2047 which represents Inf or NaN
            if (Bits - Bitplane <= EMaxes[BI] - ToleranceExp + 1) {
              Write(Bs, 1);
              Write(Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
              // TODO: padding?
              ForwardBlockTransform(&IntTile[K]);
              ForwardShuffle(&IntTile[K], &UIntTile[K]);
            } else {
              Write(Bs, 0);
            }
          }
          /* encode */
          if (Bits - Bitplane <= EMaxes[BI] - ToleranceExp + 1) {
            int& N = Ns[BI];
            EncodeBlock(&UIntTile[K], Bitplane, N, Bs);
          }
        }}} /* end loop through the zfp blocks */
      } /* end loop through the bit planes */
      /* Write the chunk header to disk (after the compressed chunk itself) */
    }}} /* end loop through the tiles */
  } /* end loop through the subbands */
  Flush(Bs);
  printf("Compressed size = %lld\n", Size(Bs));
  printf("Encoding time: %f s\n", ResetTimer(&Timer) / 1000.0);
}

void DecodeZfp(f64* Data, v3i Dims, v3i TileDims, int Bits, f64 Tolerance,
               const dynamic_array<extent>& Subbands, bitstream* Bs)
{
  // TODO: use many different bit streams
  timer Timer;
  StartTimer(&Timer);
  Rewind(Bs);
  InitRead(Bs, Bs->Stream);
  v3i BlockDims{ 4, 4, 4 };
  v3i NumBlocks = ((TileDims + BlockDims) - 1) / BlockDims;
  int ToleranceExp = Exponent(Tolerance);
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = Extract3Ints(Subbands[S].Pos);
    v3i SubbandDims = Extract3Ints(Subbands[S].Dims);
    /* loop through the tiles */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
    // for (int TZ = 0; TZ < Dims.Z; TZ += TileDims.Z) {
    // for (int TY = 0; TY < Dims.Y; TY += TileDims.Y) {
    // for (int TX = 0; TX < Dims.X; TX += TileDims.X) {
      // TODO: use the freelist allocator
      // TODO: use aligned memory allocation
      // TODO: try reusing the memory buffer
      mg_HeapArray(FloatTile, f64, Prod<i64>(TileDims));
      mg_HeapArray(IntTile, i64, Prod<i64>(TileDims));
      mg_HeapArray(UIntTile, u64, Prod<i64>(TileDims));
      memset(UIntTile.Data, 0, sizeof(u64) * Prod<i64>(TileDims));
      mg_HeapArray(Ns, int, Prod<i64>(NumBlocks));
      memset(Ns.Data, 0, sizeof(int) * Prod<i64>(NumBlocks));
      mg_HeapArray(EMaxes, int, Prod<i64>(NumBlocks));
      /* compute the total size of the chunks */
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        for (int BZ = 0; BZ < TileDims.Z; BZ += BlockDims.Z) { /* loop through zfp blocks */
        for (int BY = 0; BY < TileDims.Y; BY += BlockDims.Y) {
        for (int BX = 0; BX < TileDims.X; BX += BlockDims.X) {
          i64 BI = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims);
          if (Bitplane == Bits) {
            bool IsBlockSignificant = Read(Bs);
            if (IsBlockSignificant)
              EMaxes[BI] = Read(Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
            else
              EMaxes[BI] = ToleranceExp - 2;
          }
          i64 K = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims) * Prod<i32>(BlockDims);
          if (Bits - Bitplane <= EMaxes[BI] - ToleranceExp + 1) {
            int& N = Ns[BI];
            DecodeBlock(&UIntTile[K], Bitplane, N, Bs);
          }
          if (Bitplane == 0) { // copy data back in the last loop iteratoin
            InverseShuffle(&UIntTile[K], &IntTile[K]);
            InverseBlockTransform(&IntTile[K]);
            Dequantize(&IntTile[K], Prod<i32>(BlockDims), EMaxes[BI], Bits - 1,
                       &FloatTile[K], data_type::float64); // TODO: 64 bit planes?
            for (int Z = 0; Z < BlockDims.Z; ++Z) { /* loop through each block */
            for (int Y = 0; Y < BlockDims.Y; ++Y) {
            for (int X = 0; X < BlockDims.X; ++X) {
              i64 I = XyzToI(Dims, v3i{ TX + BX + X, TY + BY + Y, TZ + BZ + Z });
              i64 J = K + XyzToI(BlockDims, v3i{ X, Y, Z });
              Data[I] = FloatTile[J]; // copy data to the local tile buffer
            }}}
          }
        }}} /* end loop through each block */
      } // end loop through the bit planes
      // fprintf(stderr, "\n");
      // TODO: padding?
    }}} /* end loop through the tiles */
  }
  printf("Decoding time: %f s\n", ResetTimer(&Timer) / 1000.0);
}

} // namespace mg
