#pragma once

#include "mg_assert.h"

namespace mg {

/*
zfp lifting transform for 4 samples in 1D.
 non-orthogonal transform
        ( 4  4  4  4) (X)
 1/16 * ( 5  1 -1 -5) (Y)
        (-4  4  4 -4) (Z)
        (-2  6 -6  2) (W) */
// TODO: look into range expansion for this transform
mg_T(t) void 
FLift(t* P, int S) {
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

/*
zfp inverse lifting transform for 4 samples in 1D.
NOTE: this lifting is not perfectly reversible
 non-orthogonal transform
       ( 4  6 -4 -1) (x)
 1/4 * ( 4  2  4  5) (y)
       ( 4 -2  4 -5) (z)
       ( 4 -6 -4  1) (w) */
mg_T(t) void
ILift(t* P, int S) {
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

mg_T(t) void
ForwardZfp(t* P) {
  mg_Assert(P);
  /* transform along X */
  for (int Z = 0; Z < 4; ++Z)
    for (int Y = 0; Y < 4; ++Y)
      FLift(P + 4 * Y + 16 * Z, 1);
  /* transform along Y */
  for (int X = 0; X < 4; ++X)
    for (int Z = 0; Z < 4; ++Z)
      FLift(P + 16 * Z + 1 * X, 4);
  /* transform along Z */
  for (int Y = 0; Y < 4; ++Y)
    for (int X = 0; X < 4; ++X)
      FLift(P + 1 * X + 4 * Y, 16);
}

mg_T(t) void
InverseZfp(t* P) {
  mg_Assert(P);
  /* transform along Z */
  for (int Y = 0; Y < 4; ++Y)
    for (int X = 0; X < 4; ++X)
      ILift(P + 1 * X + 4 * Y, 16);
  /* transform along y */
  for (int X = 0; X < 4; ++X)
    for (int Z = 0; Z < 4; ++Z)
      ILift(P + 16 * Z + 1 * X, 4);
  /* transform along X */
  for (int Z = 0; Z < 4; ++Z)
    for (int Y = 0; Y < 4; ++Y)
      ILift(P + 4 * Y + 16 * Z, 1);
}

/*
Use the following array to reorder transformed coefficients in a zfp block.
The ordering is first by i + j + k, then by i^2 + j^2 + k^2. */
#define mg_Index(i, j, k) ((i) + 4 * (j) + 16 * (k))
constexpr i8 
Perm3[64] = {
  mg_Index(0, 0, 0), /*  0 : 0 */

  mg_Index(1, 0, 0), /*  1 : 1 */
  mg_Index(0, 1, 0), /*  2 : 1 */
  mg_Index(0, 0, 1), /*  3 : 1 */

  mg_Index(0, 1, 1), /*  4 : 2 */
  mg_Index(1, 0, 1), /*  5 : 2 */
  mg_Index(1, 1, 0), /*  6 : 2 */

  mg_Index(2, 0, 0), /*  7 : 2 */
  mg_Index(0, 2, 0), /*  8 : 2 */
  mg_Index(0, 0, 2), /*  9 : 2 */

  mg_Index(1, 1, 1), /* 10 : 3 */
  mg_Index(2, 1, 0), /* 11 : 3 */
  mg_Index(2, 0, 1), /* 12 : 3 */
  mg_Index(0, 2, 1), /* 13 : 3 */
  mg_Index(1, 2, 0), /* 14 : 3 */
  mg_Index(1, 0, 2), /* 15 : 3 */
  mg_Index(0, 1, 2), /* 16 : 3 */

  mg_Index(3, 0, 0), /* 17 : 3 */
  mg_Index(0, 3, 0), /* 18 : 3 */
  mg_Index(0, 0, 3), /* 19 : 3 */

  mg_Index(2, 1, 1), /* 20 : 4 */
  mg_Index(1, 2, 1), /* 21 : 4 */
  mg_Index(1, 1, 2), /* 22 : 4 */

  mg_Index(0, 2, 2), /* 23 : 4 */
  mg_Index(2, 0, 2), /* 24 : 4 */
  mg_Index(2, 2, 0), /* 25 : 4 */

  mg_Index(3, 1, 0), /* 26 : 4 */
  mg_Index(3, 0, 1), /* 27 : 4 */
  mg_Index(0, 3, 1), /* 28 : 4 */
  mg_Index(1, 3, 0), /* 29 : 4 */
  mg_Index(1, 0, 3), /* 30 : 4 */
  mg_Index(0, 1, 3), /* 31 : 4 */

  mg_Index(1, 2, 2), /* 32 : 5 */
  mg_Index(2, 1, 2), /* 33 : 5 */
  mg_Index(2, 2, 1), /* 34 : 5 */

  mg_Index(3, 1, 1), /* 35 : 5 */
  mg_Index(1, 3, 1), /* 36 : 5 */
  mg_Index(1, 1, 3), /* 37 : 5 */

  mg_Index(3, 2, 0), /* 38 : 5 */
  mg_Index(3, 0, 2), /* 39 : 5 */
  mg_Index(0, 3, 2), /* 40 : 5 */
  mg_Index(2, 3, 0), /* 41 : 5 */
  mg_Index(2, 0, 3), /* 42 : 5 */
  mg_Index(0, 2, 3), /* 43 : 5 */

  mg_Index(2, 2, 2), /* 44 : 6 */

  mg_Index(3, 2, 1), /* 45 : 6 */
  mg_Index(3, 1, 2), /* 46 : 6 */
  mg_Index(1, 3, 2), /* 47 : 6 */
  mg_Index(2, 3, 1), /* 48 : 6 */
  mg_Index(2, 1, 3), /* 49 : 6 */
  mg_Index(1, 2, 3), /* 50 : 6 */

  mg_Index(0, 3, 3), /* 51 : 6 */
  mg_Index(3, 0, 3), /* 52 : 6 */
  mg_Index(3, 3, 0), /* 53 : 6 */

  mg_Index(3, 2, 2), /* 54 : 7 */
  mg_Index(2, 3, 2), /* 55 : 7 */
  mg_Index(2, 2, 3), /* 56 : 7 */

  mg_Index(1, 3, 3), /* 57 : 7 */
  mg_Index(3, 1, 3), /* 58 : 7 */
  mg_Index(3, 3, 1), /* 59 : 7 */

  mg_Index(2, 3, 3), /* 60 : 8 */
  mg_Index(3, 2, 3), /* 61 : 8 */
  mg_Index(3, 3, 2), /* 62 : 8 */

  mg_Index(3, 3, 3), /* 63 : 9 */
};
#undef mg_Index

mg_T2(t, u) void
ForwardShuffle(t* IBlock, u* UBlock) {
  auto Mask = traits<u>::NBinaryMask;
  for (int I = 0; I < 64; ++I)
    UBlock[I] = (u)((IBlock[Perm3[I]] + Mask) ^ Mask);
}

mg_T2(t, u) void
InverseShuffle(u* UBlock, t* IBlock) {
  auto Mask = traits<u>::NBinaryMask;
  for (int I = 0; I < 64; ++I)
    IBlock[Perm3[I]] = (t)((UBlock[I] ^ Mask) - Mask);
}

mg_T(t) void
PadBlock(t* P, int N, int S) {
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

} // namespace mg

