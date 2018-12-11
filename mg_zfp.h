/* Adapted from the zfp compression library */

#pragma once

#include "mg_array.h"
#include "mg_assert.h"
#include "mg_bitstream.h"
#include "mg_common_types.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_signal_processing.h"

namespace mg {

/* zfp lifting transform for 4 samples in 1D.
 non-orthogonal transform
        ( 4  4  4  4) (X)
 1/16 * ( 5  1 -1 -5) (Y)
        (-4  4  4 -4) (Z)
        (-2  6 -6  2) (W) */
template <typename t>
void ForwardLift(t* P, int S) {
  mg_Assert(P);
  mg_Assert(S > 0);
  t X = P[0 * S], Y = P[1 * S], Z = P[2 * S], W = P[3 * S];
  X += W; X >>= 1; W -= X;
  Z += Y; Z >>= 1; Y -= Z;
  X += Z; X >>= 1; Z -= X;
  W += Y; W >>= 1; Y -= W;
  W += Y >> 1; Y -= W >> 1;
  P[0 * S] = X; P[1 * S] = Y; P[2 * S] = Z; P[3 * S] = W;
}

/* zfp inverse lifting transform for 4 samples in 1D. NOTE: this lifting is not perfectly reversible
 non-orthogonal transform
       ( 4  6 -4 -1) (x)
 1/4 * ( 4  2  4  5) (y)
       ( 4 -2  4 -5) (z)
       ( 4 -6 -4  1) (w) */
template <typename t>
void InverseLift(t* P, int S) {
  mg_Assert(P);
  mg_Assert(S > 0);
  t X = P[0 * S], Y = P[1 * S], Z = P[2 * S], W = P[3 * S];
  Y += W >> 1; W -= Y >> 1;
  Y += W; W <<= 1; W -= Y;
  Z += X; X <<= 1; X -= Z;
  Y += Z; Z <<= 1; Z -= Y;
  W += X; X <<= 1; X -= W;
  P[0 * S] = X; P[1 * S] = Y; P[2 * S] = Z; P[3 * S] = W;
}

/* zfp forward transform for 64 samples in 3D. The input is assumed to be in row-major order. */
template <typename t>
void ForwardBlockTransform(t* P) {
  mg_Assert(P);
  /* transform along X */
  for (int Z = 0; Z < 4; ++Z)
    for (int Y = 0; Y < 4; ++Y)
      ForwardLift(P + 4 * Y + 16 * Z, 1);
  /* transform along Y */
  for (int X = 0; X < 4; ++X)
    for (int Z = 0; Z < 4; ++Z)
      ForwardLift(P + 16 * Z + 1 * X, 4);
  /* transform along Z */
  for (int Y = 0; Y < 4; ++Y)
    for (int X = 0; X < 4; ++X)
      ForwardLift(P + 1 * X + 4 * Y, 16);
}

/* zfp inverse transform for 64 samples in 3D. The input is assumed to be in row-major order. */
template <typename t>
void InverseBlockTransform(t* P) {
  mg_Assert(P);
  /* transform along Z */
  for (int Y = 0; Y < 4; ++Y)
    for (int X = 0; X < 4; ++X)
      InverseLift(P + 1 * X + 4 * Y, 16);
  /* transform along y */
  for (int X = 0; X < 4; ++X)
    for (int Z = 0; Z < 4; ++Z)
      InverseLift(P + 16 * Z + 1 * X, 4);
  /* transform along X */
  for (int Z = 0; Z < 4; ++Z)
    for (int Y = 0; Y < 4; ++Y)
      InverseLift(P + 4 * Y + 16 * Z, 1);
}

/* Use the following array to reorder transformed coefficients in a zfp block
The ordering is first by i + j + k, then by i^2 + j^2 + k^2. */
#define index(i, j, k) ((i) + 4 * (j) + 16 * (k))
constexpr i8 Perm3[64] = {
  index(0, 0, 0), /*  0 : 0 */

  index(1, 0, 0), /*  1 : 1 */
  index(0, 1, 0), /*  2 : 1 */
  index(0, 0, 1), /*  3 : 1 */

  index(0, 1, 1), /*  4 : 2 */
  index(1, 0, 1), /*  5 : 2 */
  index(1, 1, 0), /*  6 : 2 */

  index(2, 0, 0), /*  7 : 2 */
  index(0, 2, 0), /*  8 : 2 */
  index(0, 0, 2), /*  9 : 2 */

  index(1, 1, 1), /* 10 : 3 */
  index(2, 1, 0), /* 11 : 3 */
  index(2, 0, 1), /* 12 : 3 */
  index(0, 2, 1), /* 13 : 3 */
  index(1, 2, 0), /* 14 : 3 */
  index(1, 0, 2), /* 15 : 3 */
  index(0, 1, 2), /* 16 : 3 */

  index(3, 0, 0), /* 17 : 3 */
  index(0, 3, 0), /* 18 : 3 */
  index(0, 0, 3), /* 19 : 3 */

  index(2, 1, 1), /* 20 : 4 */
  index(1, 2, 1), /* 21 : 4 */
  index(1, 1, 2), /* 22 : 4 */

  index(0, 2, 2), /* 23 : 4 */
  index(2, 0, 2), /* 24 : 4 */
  index(2, 2, 0), /* 25 : 4 */

  index(3, 1, 0), /* 26 : 4 */
  index(3, 0, 1), /* 27 : 4 */
  index(0, 3, 1), /* 28 : 4 */
  index(1, 3, 0), /* 29 : 4 */
  index(1, 0, 3), /* 30 : 4 */
  index(0, 1, 3), /* 31 : 4 */

  index(1, 2, 2), /* 32 : 5 */
  index(2, 1, 2), /* 33 : 5 */
  index(2, 2, 1), /* 34 : 5 */

  index(3, 1, 1), /* 35 : 5 */
  index(1, 3, 1), /* 36 : 5 */
  index(1, 1, 3), /* 37 : 5 */

  index(3, 2, 0), /* 38 : 5 */
  index(3, 0, 2), /* 39 : 5 */
  index(0, 3, 2), /* 40 : 5 */
  index(2, 3, 0), /* 41 : 5 */
  index(2, 0, 3), /* 42 : 5 */
  index(0, 2, 3), /* 43 : 5 */

  index(2, 2, 2), /* 44 : 6 */

  index(3, 2, 1), /* 45 : 6 */
  index(3, 1, 2), /* 46 : 6 */
  index(1, 3, 2), /* 47 : 6 */
  index(2, 3, 1), /* 48 : 6 */
  index(2, 1, 3), /* 49 : 6 */
  index(1, 2, 3), /* 50 : 6 */

  index(0, 3, 3), /* 51 : 6 */
  index(3, 0, 3), /* 52 : 6 */
  index(3, 3, 0), /* 53 : 6 */

  index(3, 2, 2), /* 54 : 7 */
  index(2, 3, 2), /* 55 : 7 */
  index(2, 2, 3), /* 56 : 7 */

  index(1, 3, 3), /* 57 : 7 */
  index(3, 1, 3), /* 58 : 7 */
  index(3, 3, 1), /* 59 : 7 */

  index(2, 3, 3), /* 60 : 8 */
  index(3, 2, 3), /* 61 : 8 */
  index(3, 3, 2), /* 62 : 8 */

  index(3, 3, 3), /* 63 : 9 */
};
#undef index

/* Reorder signed coefficients within a zfp block, and convert them to nega-binary */
template <typename t, typename u>
void ForwardShuffle(const t* IBlock, u* UBlock) {
  for (int I = 0; I < 64; ++I) {
    auto Mask = Traits<u>::NegabinaryMask;
    UBlock[I] = (u)((IBlock[Perm3[I]] + Mask) ^ Mask);
  }
}

/* Reorder unsigned coefficients within a block, and convert them to two's complement */
template <typename t, typename u>
void InverseShuffle(const u* UBlock, t* IBlock) {
  for (int I = 0; I < 64; ++I) {
    auto Mask = Traits<u>::NegabinaryMask;
    IBlock[Perm3[I]] = (t)((UBlock[I] ^ Mask) - Mask);
  }
}

/* Pad partial block of width N < 4 and stride S */
template <typename t>
void PadBlock(t* P, int N, int S) {
  mg_Assert(P);
  mg_Assert(0 <= N && N <= 4);
  mg_Assert(S > 0);
  switch (N) {
  case 0:
    P[0 * S] = 0; /* fall through */
  case 1:
    P[1 * S] = P[0 * S]; /* fall through */
  case 2:
    P[2 * S] = P[1 * S]; /* fall through */
  case 3:
    P[3 * S] = P[0 * S]; /* fall through */
  default:
    break;
  }
}

/* Encode a single bit plane of a single zfp block */
void EncodeBlock(const u64* Block, int Bitplane, int& N, bit_stream* Bs) {
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

/* Decode a single bit plane of a single zfp block */
// TODO: pointer aliasing?
void DecodeBlock(u64* Block, int Bitplane, int& N, bit_stream* Bs) {
  /* decode first N bits of bit plane #Bitplane */
  u64 X = ReadLong(Bs, N);
  /* unary run-length decode remainder of bit plane */
  for (; N < 64 && Read(Bs); X += u64(1) << N++)
    for (; N < 64 - 1 && !Read(Bs); ++N) ;
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
}

/* Encode a region of data */
inline // TODO: turn this function into a template ?
void EncodeData(const f64* Data, v3i Dims, v3i TileDims, bit_stream* Bs) {
  // TODO: loop through the subbands
  // TODO: use many different bit streams
  // TODO: initialize bit stream?
  // TODO: error handling
  FILE* Fp = fopen("compressed.raw", "wb");
  v3i BlockDims{ 4, 4, 4 };
  v3i NumTiles = (Dims + TileDims - 1) / TileDims;
  fseek(Fp, sizeof(size_t) * Prod<i64>(NumTiles), SEEK_SET); // reserve space for the tile pointers
  v3i NumBlocks = ((TileDims + BlockDims) - 1) / BlockDims;
  for (int TZ = 0; TZ < Dims.Z; TZ += TileDims.Z) { /* loop through the tiles */
  for (int TY = 0; TY < Dims.Y; TY += TileDims.Y) {
  for (int TX = 0; TX < Dims.X; TX += TileDims.X) {
    i64 TileId = XyzToI(NumTiles, v3i{ TX, TY, TZ } / TileDims);
    // TODO: use the freelist allocator
    // TODO: use aligned memory allocation
    // TODO: try reusing the memory buffer
    f64* FloatTile = nullptr; Allocate((byte**)&FloatTile, sizeof(f64) * Prod<i64>(TileDims));
    i64* IntTile = nullptr; Allocate((byte**)&IntTile, sizeof(i64) * Prod<i64>(TileDims));
    u64* UIntTile = nullptr; Allocate((byte**)&UIntTile, sizeof(u64) * Prod<i64>(TileDims));
    int* Ns = nullptr; Allocate((byte**)&Ns, sizeof(int) * Prod<i64>(NumBlocks));
    memset(Ns, 0, sizeof(int) * Prod<i64>(NumBlocks));
    short ChunkBytes[64] = { 0 }; // store the size of the chunks in bytes (at mots we have 64 chunks)
    int ChunkId = 0;
    for (int Bitplane = 63; Bitplane >= 0; --Bitplane) {
      for (int BZ = 0; BZ < TileDims.Z; BZ += BlockDims.Z) { /* loop through zfp blocks */
      for (int BY = 0; BY < TileDims.Y; BY += BlockDims.Y) {
      for (int BX = 0; BX < TileDims.X; BX += BlockDims.X) {
        i64 K = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims) * Prod<i32>(BlockDims); // TODO: handle partial blocks
        if (Bitplane == 63) { // only do the following on the first loop iteration
          for (int Z = 0; Z < BlockDims.Z; ++Z) { /* loop through each block */
          for (int Y = 0; Y < BlockDims.Y; ++Y) {
          for (int X = 0; X < BlockDims.X; ++X) {
              i64 I = XyzToI(Dims, v3i{ TX + BX + X, TY + BY + Y, TZ + BZ + Z });
              i64 J = K + XyzToI(BlockDims, v3i{ X, Y, Z });
              FloatTile[J] = Data[I]; // copy data to the local tile buffer
          }}} /* end loop through each block */
          int EMax = Quantize(&FloatTile[K], Prod<i32>(BlockDims), 52, &IntTile[K], data_type::float64); // TODO: 64 bit planes?
          // TODO: for now we don't care if the exponent is 2047 which represents Inf or NaN
          Write(Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
          // TODO: padding?
          ForwardBlockTransform(&IntTile[K]);
          ForwardShuffle(&IntTile[K], &UIntTile[K]);
        }
        /* encode */
        i64 BlockId = XyzToI(NumBlocks, v3i{ BX, BY, BZ } / BlockDims);
        int& N = Ns[BlockId];
        EncodeBlock(&UIntTile[K], Bitplane, N, Bs);
        size_t Bytes = Size(Bs);
        if (BlockId + 1 == Prod<i32>(NumBlocks)) { // last block in the tile
          if (Bytes >= 4096 || Bitplane == 0) {
            Flush(Bs);
            mg_Assert(Size(Bs) == Bytes);
            fprintf(stderr, "%d ", ChunkId);
            ChunkBytes[ChunkId++] = Bytes;
            fprintf(stderr, "(%ld)", ftell(Fp));
            fwrite(Bs->Stream.Data, Bytes, 1, Fp);
            InitWrite(Bs, Bs->Stream);
            //printf("%llu ", Bytes);
          }
        }
      }}} /* end loop through the zfp blocks */
    } // end loop through the bit planes
    fprintf(stderr, "\n");
    mg_Assert(ChunkId <= 64);
    size_t BeginChunkHeaders = ftell(Fp);
    fwrite(&ChunkId, sizeof(ChunkId), 1, Fp); // number of chunks
    fwrite(ChunkBytes, sizeof(ChunkBytes[0]) * ChunkId, 1, Fp); // write the chunk sizes
    size_t EndChunkHeaders = ftell(Fp);
    fseek(Fp, sizeof(size_t) * TileId, SEEK_SET); // back to tile headers
    fwrite(&BeginChunkHeaders, sizeof(BeginChunkHeaders), 1, Fp); // write the pointer to the current chunk
    fseek(Fp, EndChunkHeaders, SEEK_SET); // continue where we left off

    Deallocate((byte**)&FloatTile);
    Deallocate((byte**)&IntTile);
    Deallocate((byte**)&UIntTile);
    Deallocate((byte**)&Ns);
  }}} /* end loop through the tiles */
  Flush(Bs);
  fclose(Fp);
}

inline // TODO: turn this function into a template ?
void DecodeData(f64* Data, v3i Dims, v3i TileDims) {
  // TODO: use many different bit streams
  FILE* Fp = fopen("compressed.raw", "rb");
  v3i BlockDims{ 4, 4, 4 };
  v3i NumTiles = (Dims + TileDims - 1) / TileDims;
  // fseek(Fp, sizeof(void*) * Prod<i64>(NumTiles), SEEK_SET); // reserve space for the tile pointers
  size_t* TilePointers = nullptr; Allocate((byte**)&TilePointers, sizeof(size_t) * Prod<i64>(NumTiles));
  fread(TilePointers, sizeof(size_t) * Prod<i64>(NumTiles), 1, Fp);
  v3i NumBlocks = ((TileDims + BlockDims) - 1) / BlockDims;
  for (int TZ = 0; TZ < Dims.Z; TZ += TileDims.Z) { /* loop through the tiles */
  for (int TY = 0; TY < Dims.Y; TY += TileDims.Y) {
  for (int TX = 0; TX < Dims.X; TX += TileDims.X) {
    i64 TileId = XyzToI(NumTiles, v3i{ TX, TY, TZ } / TileDims);
    // TODO: use the freelist allocator
    // TODO: use aligned memory allocation
    // TODO: try reusing the memory buffer
    f64* FloatTile = nullptr; Allocate((byte**)&FloatTile, sizeof(f64) * Prod<i64>(TileDims));
    i64* IntTile = nullptr; Allocate((byte**)&IntTile, sizeof(i64) * Prod<i64>(TileDims));
    u64* UIntTile = nullptr; Allocate((byte**)&UIntTile, sizeof(u64) * Prod<i64>(TileDims));
    memset(UIntTile, 0, sizeof(u64) * Prod<i64>(TileDims));
    int* Ns = nullptr; Allocate((byte**)&Ns, sizeof(int) * Prod<i64>(NumBlocks));
    memset(Ns, 0, sizeof(int) * Prod<i64>(NumBlocks));
    int* EMaxes = nullptr; Allocate((byte**)&EMaxes, sizeof(int) * Prod<i64>(NumBlocks));
    memset(Ns, 0, sizeof(int) * Prod<i64>(NumBlocks));
    fseek(Fp, TilePointers[TileId], SEEK_SET);
    int NumChunks = 0;
    fread(&NumChunks, sizeof(NumChunks), 1, Fp);
    short ChunkBytes[64] = { 0 };
    fread(ChunkBytes, sizeof(ChunkBytes[0]) * NumChunks, 1, Fp);
    /* compute the total size of the chunks */
    size_t TotalChunkSize = 0;
    for (int I = 0; I < NumChunks; ++I) {
      TotalChunkSize += ChunkBytes[I];
    }
    fseek(Fp, TilePointers[TileId] - TotalChunkSize, SEEK_SET);
    dynamic_array<byte> Buf;
    Resize(&Buf, ChunkBytes[0]);
    fprintf(stderr, "0 (%ld)", ftell(Fp));
    fread(Buf.Buffer.Data, ChunkBytes[0], 1, Fp);
    bit_stream Bs;
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
          Dequantize(&IntTile[K], Prod<i32>(BlockDims), EMaxes[BlockId], 52, &FloatTile[K], data_type::float64); // TODO: 64 bit planes?
          for (int Z = 0; Z < BlockDims.Z; ++Z) { /* loop through each block */
          for (int Y = 0; Y < BlockDims.Y; ++Y) {
          for (int X = 0; X < BlockDims.X; ++X) {
            i64 I = XyzToI(Dims, v3l{ TX + BX + X, TY + BY + Y, TZ + BZ + Z });
            i64 J = K + XyzToI(BlockDims, v3i{ X, Y, Z });
            Data[I] = FloatTile[J]; // copy data to the local tile buffer
          }}}
        }
        /* read the next chunk if necessary */
        if (BlockId + 1 == Prod<i32>(NumBlocks)) {
          if (Size(&Bs) >= 4096 || Bitplane == 0) {
            if (ChunkId < NumChunks - 1) {
              fprintf(stderr, "%d ", ChunkId + 1);
              Resize(&Buf, ChunkBytes[++ChunkId]);
              fprintf(stderr, "(%ld)", ftell(Fp));
              fread(Buf.Buffer.Data, ChunkBytes[ChunkId], 1, Fp);
              InitRead(&Bs, Buf.Buffer);
            }
          }
        }
      }}} /* end loop through each block */
    } // end loop through the bit planes
    fprintf(stderr, "\n");
    // TODO: padding?
    Deallocate((byte**)&FloatTile);
    Deallocate((byte**)&IntTile);
    Deallocate((byte**)&UIntTile);
    Deallocate((byte**)&Ns);
    Deallocate((byte**)&EMaxes);
  }}} /* end loop through the tiles */
  Deallocate((byte**)&TilePointers);
  fclose(Fp);
}

} // namespace mg
