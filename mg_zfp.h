/* Adapted from the zfp compression library */

#pragma once

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
  for (int Z = 0; Z < 4; Z++)
    for (int Y = 0; Y < 4; Y++)
      ForwardLift(P + 4 * Y + 16 * Z, 1);
  /* transform along Y */
  for (int X = 0; X < 4; X++)
    for (int Z = 0; Z < 4; Z++)
      ForwardLift(P + 16 * Z + 1 * X, 4);
  /* transform along Z */
  for (int Y = 0; Y < 4; Y++)
    for (int X = 0; X < 4; X++)
      ForwardLift(P + 1 * X + 4 * Y, 16);
}

/* zfp inverse transform for 64 samples in 3D. The input is assumed to be in row-major order. */
template <typename t>
void InverseBlockTransform(t* P) {
  mg_Assert(P);
  /* transform along Z */
  for (int Y = 0; y < 4; y++)
    for (int X = 0; X < 4; X++)
      InverseLift(P + 1 * X + 4 * y, 16);
  /* transform along y */
  for (int X = 0; X < 4; X++)
    for (int Z = 0; Z < 4; Z++)
      InverseLift(P + 16 * Z + 1 * X, 4);
  /* transform along X */
  for (int Z = 0; Z < 4; Z++)
    for (int y = 0; y < 4; y++)
      InverseLift(P + 4 * y + 16 * Z, 1);
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
template <typename t>
void InverseShuffle(const t* UBlock, t* IBlock) {
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
  Write(Bs, X, N);
  for (; N < 64 && Write(Bs, !!X); X >>= 1, ++N)
    for (; N < 64 - 1 && !Write(Bs, X & 1u); X >>= 1, ++N) ;
}

/* Decode a single bit plane of a single zfp block */
// TODO: pointer aliasing?
void DecodeBlock(u64* Block, int Bitplane, int& N, bit_stream* Bs) {
  /* decode first N bits of bit plane #Bitplane */
  u64 X = 0;
  Read(Bs, N);
  /* unary run-length decode remainder of bit plane */
  for (; N < 64 && Read(Bs); X += (u64)1 << N++)
    for (; N < 64 - 1 && !Read(Bs); ++N) ;
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
}

/* Encode a region of data */
inline // TODO: turn this function into a template ?
void EncodeRegion(const f64* Data, v3i Dims, v3i TileDims, bit_stream* Bs) {
  // TODO: use many different bit streams
  // TODO: initialize bit stream?
  InitWrite(Bs, sizeof(f64) * Prod(Dims));
  for (int TZ = 0; TZ < Dims.Z; TZ += TileDims.Z) { /* loop through the tiles */
  for (int TY = 0; TY < Dims.Y; TY += TileDims.Y) {
  for (int TX = 0; TX < Dims.X; TX += TileDims.X) {
    // TODO: use the freelist allocator
    // TODO: use aligned memory allocation
    // TODO: try reusing the memory buffer
    f64* FloatTile; Allocate((byte**)&FloatTile, sizeof(f64) * Prod(TileDims));
    i64* IntTile; Allocate((byte**)&IntTile, sizeof(i64) * Prod(TileDims));
    u64* UIntTile; Allocate((byte**)&UIntTile, sizeof(u64) * Prod(TileDims));
    for (int BZ = 0; BZ < TileDims.Z; BZ += 4) { /* loop through zfp blocks */
    for (int BY = 0; BY < TileDims.Y; BY += 4) {
    for (int BX = 0; BX < TileDims.X; BX += 4) {
      i64 K = XyzToI(TileDims, v3l{ BX, BY, BZ });
      for (int Z = 0; Z < 4; ++Z) { /* loop through each block */
      for (int Y = 0; Y < 4; ++Y) {
      for (int X = 0; X < 4; ++X) {
        i64 I = XyzToI(Dims, v3l{ TX + BX + X, TY + BY + Y, TZ + BZ + Z });
        i64 J = K + XyzToI(v3l{ 4, 4, 4 }, v3l{ X, Y, Z });
        FloatTile[J] = Data[I]; // copy data to the local tile buffer
      }}} /* end loop through each block */
      int EMax = Quantize(&FloatTile[K], 4 * 4 * 4, 64, &IntTile[K], data_type::float64); // TODO: 64 bit planes?
      Write(Bs, EMax * 2 + 1, Traits<f64>::ExponentBits + 1);
      ForwardBlockTransform(&IntTile[K]);
      ForwardShuffle(&IntTile[K], &UIntTile[K]);
      int N = 0;
      for (int Bitplane = 0; Bitplane < 64; ++Bitplane) // TODO: move this loop outside
        EncodeBlock(&UIntTile[K], Bitplane, N, Bs);
    }}} /* end loop through the zfp blocks */
    // TODO: padding?
    Deallocate((byte**)&FloatTile);
    Deallocate((byte**)&IntTile);
    Deallocate((byte**)&UIntTile);
  }}} /* end loop through the tiles */
  Flush(Bs);
}

} // namespace mg
