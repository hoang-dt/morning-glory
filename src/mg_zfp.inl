#pragma once

#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common.h"
#include "mg_math.h"
#include <iostream>

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

mg_TI(t, S) void
ForwardZfp2D(t* P) {
  mg_Assert(P);
  /* transform along X */
  for (int Y = 0; Y < S; ++Y)
    FLift(P + S * Y, 1);
  /* transform along Y */
  for (int X = 0; X < S; ++X)
    FLift(P + 1 * X, S);
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

mg_TI(t, S) void
InverseZfp2D(t* P) {
  mg_Assert(P);
  /* transform along y */
  for (int X = 0; X < S; ++X)
    ILift(P + 1 * X, S);
  /* transform along X */
  for (int Y = 0; Y < S; ++Y)
    ILift(P + S * Y, 1);
}

mg_I(S)
struct perm2 {
inline static const stack_array<int, S * S> Table = []() {
  stack_array<int, S * S> Arr;
  int I = 0;
  for (int Y = 0; Y < S; ++Y) {
    for (int X = 0; X < S; ++X) {
      Arr[I++] = Y * S + X;
    }
  }
  for (I = 0; I < Size(Arr); ++I) {
    for (int J = I + 1; J < Size(Arr); ++J) {
      int XI = Arr[I] % S, YI = Arr[I] / S;
      int XJ = Arr[J] % S, YJ = Arr[J] / S;
      if (XI + YI > XJ + YJ) {
        Swap(&Arr[I], &Arr[J]);
      } else if ((XI + YI == XJ + YJ) && (XI * XI + YI * YI > XJ * XJ + YJ * YJ)) {
        Swap(&Arr[I], &Arr[J]);
      }
    }
  }
  return Arr;
}();
};

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

mg_TT(t, u) void
ForwardShuffle(t* IBlock, u* UBlock) {
  auto Mask = traits<u>::NBinaryMask;
  for (int I = 0; I < 64; ++I)
    UBlock[I] = (u)((IBlock[Perm3[I]] + Mask) ^ Mask);
}

mg_TTI(t, u, S) void
ForwardShuffle2D(t* IBlock, u* UBlock) {
  auto Mask = traits<u>::NBinaryMask;
  for (int I = 0; I < S * S; ++I)
    UBlock[I] = (u)((IBlock[perm2<S>::Table[I]] + Mask) ^ Mask);
}

mg_TT(t, u) void
InverseShuffle(u* UBlock, t* IBlock) {
  auto Mask = traits<u>::NBinaryMask;
  for (int I = 0; I < 64; ++I)
    IBlock[Perm3[I]] = (t)((UBlock[I] ^ Mask) - Mask);
}

mg_TTI(t, u, S) void
InverseShuffle2D(u* UBlock, t* IBlock) {
  auto Mask = traits<u>::NBinaryMask;
  for (int I = 0; I < S * S; ++I)
    IBlock[perm2<S>::Table[I]] = (t)((UBlock[I] ^ Mask) - Mask);
}

// TODO: this function is only correct for block size 4
mg_T(t) void
PadBlock1D(t* P, int N, int S) {
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

mg_T(t) void
PadBlock3D(t* P, const v3i& N) {
  for (int Z = 0; Z < 4; ++Z)
    for (int Y = 0; Y < 4; ++Y)
      PadBlock1D(P + Z * 16 + Y * 4, N.X, 1);

  for (int Z = 0; Z < 4; ++Z)
    for (int X = 0; X < 4; ++X)
      PadBlock1D(P + Z * 16 + X * 1, N.Y, 4);

  for (int Y = 0; Y < 4; ++Y)
    for (int X = 0; X < 4; ++X)
      PadBlock1D(P + Y * 4 + X * 1, N.Z, 16);
}

mg_T(t) void
PadBlock2D(t* P, const v2i& N) {
  for (int Y = 0; Y < 4; ++Y)
    PadBlock1D(P + Y * 4, N.X, 1);

  for (int X = 0; X < 4; ++X)
    PadBlock1D(P + X * 1, N.Y, 4);
}


// D is the dimension, K is the size of the block
mg_TII(t, D, K) void
Encode(t* Block, int B, i64 S, i8& N, bitstream* Bs) {
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  u64 X = 0;
  for (int I = 0; I < NVals; ++I)
    X += u64((Block[I] >> B) & 1u) << I;
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  if (P > 0) {
    WriteLong(Bs, X, P);
    X >>= P; // P == 64 is fine since in that case we don't need X any more
  }
  // TODO: we may be able to speed this up by getting rid of the shift of X
  // or the call bit BitSize()
  for (; BitSize(*Bs) < S && N < NVals;) {
    if (Write(Bs, !!X)) { // group is significant
      for (; BitSize(*Bs) < S && N + 1 < NVals;) {
        if (Write(Bs, X & 1u)) { // found a significant coeff, break and retest
          break;
        } else { // have not found a significant coeff, continue until we find one
          X >>= 1;
          ++N;
        }
      }
      if (BitSize(*Bs) >= S)
        break;
      X >>= 1;
      ++N;
    } else {
      break;
    }
  }
}

int MyCounter = 0;
mg_TII(t, D, K) void
Decode(t* Block, int B, i64 S, i8& N, bitstream* Bs) {
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  //std::cout << "Counter " << MyCounter << "P = " << int(P) << " X = " << X << " N = " << int(N) << std::endl;
  for (; /*BitSize(*Bs) < S &&*/ N < NVals;) {
    if (Read(Bs)) {
      for (; /*BitSize(*Bs) < S &&*/ N + 1 < NVals;) {
        if (Read(Bs)) {
          break;
        } else {
          ++N;
        }
      }
      /*if (BitSize(*Bs) >= S)
        break;*/
      X += 1ull << (N++);
    } else {
      break;
    }
  }
  //std::cout << "N = " << int(N) << std::endl;
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (t)(X & 1u) << B;
  //__m256i Add = _mm256_set1_epi32(t(1) << B); // TODO: should be epi64 with t == u64
  //__m256i Mask = _mm256_set_epi32(0xffffff7f, 0xffffffbf, 0xffffffdf, 0xffffffef, 0xfffffff7, 0xfffffffb, 0xfffffffd, 0xfffffffe);
  //while (X) {
  //  __m256i Val = _mm256_set1_epi32(X);
  //  Val = _mm256_or_si256(Val, Mask);
  //  Val = _mm256_cmpeq_epi32(Val, _mm256_set1_epi64x(-1));
  //  //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
  //  _mm256_maskstore_epi32((int*)Block, Val, _mm256_add_epi32(_mm256_maskload_epi32((int*)Block, Val), Add));
  //  X >>= 8;
  //  Block += 8;
  //}
  ++MyCounter;
}

mg_TII(t, D, K) void
Decode4(t* Block, int B, i64 S, i8& N, bitstream* Bs) {
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  //std::cout << "Counter " << MyCounter << "P = " << int(P) << " X = " << X << " N = " << int(N) << std::endl;
  for (; /*BitSize(*Bs) < S &&*/ N < NVals;) {
    if (Read(Bs)) {
      for (; /*BitSize(*Bs) < S &&*/ N + 1 < NVals;) {
        if (Read(Bs)) {
          break;
        } else {
          ++N;
        }
      }
      /*if (BitSize(*Bs) >= S)
        break;*/
      X += 1ull << (N++);
    } else {
      break;
    }
  }
  //std::cout << "N = " << int(N) << std::endl;
  /* deposit bit plane from x */
  //for (int I = 0; X; ++I, X >>= 1)
  //  Block[I] += (t)(X & 1u) << B;
  __m256i Add = _mm256_set1_epi32(t(1) << B); // TODO: should be epi64 with t == u64
  __m256i Mask = _mm256_set_epi32(0xffffff7f, 0xffffffbf, 0xffffffdf, 0xffffffef, 0xfffffff7, 0xfffffffb, 0xfffffffd, 0xfffffffe);
  while (X) {
    __m256i Val = _mm256_set1_epi32(X);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi32(Val, _mm256_set1_epi64x(-1));
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi32((int*)Block, Val, _mm256_add_epi32(_mm256_maskload_epi32((int*)Block, Val), Add));
    X >>= 8;
    Block += 8;
  }
  ++MyCounter;
}

mg_Ti(t) void
TransposeAvx2(u64 X, int B, t* mg_Restrict Block) {
  __m256i Minus1 = _mm256_set1_epi64x(-1);
  __m256i Add = _mm256_set1_epi64x(t(1) << B); // TODO: should be epi64 with t == u64
  __m256i Mask = _mm256_set_epi64x(0xfffffffffffffff7ll, 0xfffffffffffffffbll, 0xfffffffffffffffdll, 0xfffffffffffffffell);
  //while (X) {
    __m256i Val = _mm256_set1_epi64x(X);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block, Val), Add));
    //X >>= 4;
    //Block += 4;

    Val = _mm256_set1_epi64x(X >> 4);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 4, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 4, Val), Add));
    //X >>= 4;
    //Block += 4;

    Val = _mm256_set1_epi64x(X >> 8);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 8, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 8, Val), Add));

    Val = _mm256_set1_epi64x(X >> 12);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 12, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 12, Val), Add));

    Val = _mm256_set1_epi64x(X >> 16);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 16, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 16, Val), Add));

    Val = _mm256_set1_epi64x(X >> 20);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 20, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 20, Val), Add));

    Val = _mm256_set1_epi64x(X >> 24);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 24, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 24, Val), Add));

    Val = _mm256_set1_epi64x(X >> 28);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 28, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 28, Val), Add));

    Val = _mm256_set1_epi64x(X >> 32);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 32, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 32, Val), Add));

    Val = _mm256_set1_epi64x(X >> 36);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 36, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 36, Val), Add));

    Val = _mm256_set1_epi64x(X >> 40);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 40, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 40, Val), Add));

    Val = _mm256_set1_epi64x(X >> 44);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 44, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 44, Val), Add));

    Val = _mm256_set1_epi64x(X >> 48);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 48, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 48, Val), Add));

    Val = _mm256_set1_epi64x(X >> 52);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 52, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 52, Val), Add));

    Val = _mm256_set1_epi64x(X >> 56);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 56, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 56, Val), Add));

    Val = _mm256_set1_epi64x(X >> 60);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi64(Val, Minus1);
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi64((i64*)Block + 60, Val, _mm256_add_epi64(_mm256_maskload_epi64((i64*)Block + 60, Val), Add));
  //}
}

mg_Ti(t) void
TransposeNormal(u64 X, int B, t* mg_Restrict Block) {
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (t)(X & 1u) << B;
}

#define zfp_swap(x, y, l) \
  do { \
    const uint64 m[] = { \
      0x5555555555555555ul, \
      0x3333333333333333ul, \
      0x0f0f0f0f0f0f0f0ful, \
      0x00ff00ff00ff00fful, \
      0x0000ffff0000fffful, \
      0x00000000fffffffful, \
    }; \
    uint s = 1u << (l); \
    uint64 t = ((x) ^ ((y) >> s)) & m[(l)]; \
    (x) ^= t; \
    (y) ^= t << s; \
  } while (0)

/* compress sequence of 4^3 = 64 unsigned integers */
mg_Ti(t) void
TransposeRecursive(const t* input, t* mg_Restrict data) {
  /* working space for 64 bit planes */
  uint64 a00 = input[0] , a01 = input[1] , a02 = input[2] , a03 = input[3] , a04 = input[4] ;
  uint64 a05 = input[5] , a06 = input[6] , a07 = input[7] , a08 = input[8] , a09 = input[9] ;
  uint64 a0a = input[10], a0b = input[11], a0c = input[12], a0d = input[13], a0e = input[14];
  uint64 a0f = input[15], a10 = input[16], a11 = input[17], a12 = input[18], a13 = input[19];
  uint64 a14 = input[20], a15 = input[21], a16 = input[22], a17 = input[23], a18 = input[24];
  uint64 a19 = input[25], a1a = input[26], a1b = input[27], a1c = input[28], a1d = input[29];
  uint64 a1e = input[30], a1f = input[31], a20 = input[32], a21 = input[33], a22 = input[34];
  uint64 a23 = input[35], a24 = input[36], a25 = input[37], a26 = input[38], a27 = input[39];
  uint64 a28 = input[40], a29 = input[41], a2a = input[42], a2b = input[43], a2c = input[44];
  uint64 a2d = input[45], a2e = input[46], a2f = input[47], a30 = input[48], a31 = input[49];
  uint64 a32 = input[50], a33 = input[51], a34 = input[52], a35 = input[53], a36 = input[54];
  uint64 a37 = input[55], a38 = input[56], a39 = input[57], a3a = input[58], a3b = input[59];
  uint64 a3c = input[60], a3d = input[61], a3e = input[62], a3f = input[63];

  /* 00, 01 */
  //encode_bit_plane(a3f);
  //encode_bit_plane(a3e);

  zfp_swap(a3e, a3f, 0);
  /* 3e, 3f */

  //encode_bit_plane(a3d);
  //encode_bit_plane(a3c);

  zfp_swap(a3c, a3d, 0);

  zfp_swap(a3d, a3f, 1);
  zfp_swap(a3c, a3e, 1);
  /* 3c, 3d */

  //encode_bit_plane(a3b);
  //encode_bit_plane(a3a);

  zfp_swap(a3a, a3b, 0);
  /* 3a, 3b */

  //encode_bit_plane(a39);
  //encode_bit_plane(a38);

  zfp_swap(a38, a39, 0);

  zfp_swap(a39, a3b, 1);
  zfp_swap(a38, a3a, 1);

  zfp_swap(a3b, a3f, 2);
  zfp_swap(a3a, a3e, 2);
  zfp_swap(a39, a3d, 2);
  zfp_swap(a38, a3c, 2);
  /* 38, 39 */

  //encode_bit_plane(a37);
  //encode_bit_plane(a36);

  zfp_swap(a36, a37, 0);
  /* 36, 37 */

  //encode_bit_plane(a35);
  //encode_bit_plane(a34);

  zfp_swap(a34, a35, 0);

  zfp_swap(a35, a37, 1);
  zfp_swap(a34, a36, 1);
  /* 34, 35 */

  //encode_bit_plane(a33);
  //encode_bit_plane(a32);

  zfp_swap(a32, a33, 0);
  /* 32, 33 */

  //encode_bit_plane(a31);
  //encode_bit_plane(a30);

  zfp_swap(a30, a31, 0);

  zfp_swap(a31, a33, 1);
  zfp_swap(a30, a32, 1);

  zfp_swap(a33, a37, 2);
  zfp_swap(a32, a36, 2);
  zfp_swap(a31, a35, 2);
  zfp_swap(a30, a34, 2);

  zfp_swap(a37, a3f, 3);
  zfp_swap(a36, a3e, 3);
  zfp_swap(a35, a3d, 3);
  zfp_swap(a34, a3c, 3);
  zfp_swap(a33, a3b, 3);
  zfp_swap(a32, a3a, 3);
  zfp_swap(a31, a39, 3);
  zfp_swap(a30, a38, 3);
  /* 30, 31 */

  //encode_bit_plane(a2f);
  //encode_bit_plane(a2e);

  zfp_swap(a2e, a2f, 0);
  /* 2e, 2f */

  //encode_bit_plane(a2d);
  //encode_bit_plane(a2c);

  zfp_swap(a2c, a2d, 0);

  zfp_swap(a2d, a2f, 1);
  zfp_swap(a2c, a2e, 1);
  /* 2c, 2d */

  //encode_bit_plane(a2b);
  //encode_bit_plane(a2a);

  zfp_swap(a2a, a2b, 0);
  /* 2a, 2b */

  //encode_bit_plane(a29);
  //encode_bit_plane(a28);

  zfp_swap(a28, a29, 0);

  zfp_swap(a29, a2b, 1);
  zfp_swap(a28, a2a, 1);

  zfp_swap(a2b, a2f, 2);
  zfp_swap(a2a, a2e, 2);
  zfp_swap(a29, a2d, 2);
  zfp_swap(a28, a2c, 2);
  /* 28, 29 */

  //encode_bit_plane(a27);
  //encode_bit_plane(a26);

  zfp_swap(a26, a27, 0);
  /* 26, 27 */

  //encode_bit_plane(a25);
  //encode_bit_plane(a24);

  zfp_swap(a24, a25, 0);

  zfp_swap(a25, a27, 1);
  zfp_swap(a24, a26, 1);
  /* 24, 25 */

  //encode_bit_plane(a23);
  //encode_bit_plane(a22);

  zfp_swap(a22, a23, 0);
  /* 22, 23 */

  //encode_bit_plane(a21);
  //encode_bit_plane(a20);

  zfp_swap(a20, a21, 0);

  zfp_swap(a21, a23, 1);
  zfp_swap(a20, a22, 1);

  zfp_swap(a23, a27, 2);
  zfp_swap(a22, a26, 2);
  zfp_swap(a21, a25, 2);
  zfp_swap(a20, a24, 2);

  zfp_swap(a27, a2f, 3);
  zfp_swap(a26, a2e, 3);
  zfp_swap(a25, a2d, 3);
  zfp_swap(a24, a2c, 3);
  zfp_swap(a23, a2b, 3);
  zfp_swap(a22, a2a, 3);
  zfp_swap(a21, a29, 3);
  zfp_swap(a20, a28, 3);

  zfp_swap(a2f, a3f, 4);
  zfp_swap(a2e, a3e, 4);
  zfp_swap(a2d, a3d, 4);
  zfp_swap(a2c, a3c, 4);
  zfp_swap(a2b, a3b, 4);
  zfp_swap(a2a, a3a, 4);
  zfp_swap(a29, a39, 4);
  zfp_swap(a28, a38, 4);
  zfp_swap(a27, a37, 4);
  zfp_swap(a26, a36, 4);
  zfp_swap(a25, a35, 4);
  zfp_swap(a24, a34, 4);
  zfp_swap(a23, a33, 4);
  zfp_swap(a22, a32, 4);
  zfp_swap(a21, a31, 4);
  zfp_swap(a20, a30, 4);
  /* 20, 21 */

  //encode_bit_plane(a1f);
  //encode_bit_plane(a1e);

  zfp_swap(a1e, a1f, 0);
  /* 1e, 1f */

  //encode_bit_plane(a1d);
  //encode_bit_plane(a1c);

  zfp_swap(a1c, a1d, 0);

  zfp_swap(a1d, a1f, 1);
  zfp_swap(a1c, a1e, 1);
  /* 1c, 1d */

  //encode_bit_plane(a1b);
  //encode_bit_plane(a1a);

  zfp_swap(a1a, a1b, 0);
  /* 1a, 1b */

  //encode_bit_plane(a19);
  //encode_bit_plane(a18);

  zfp_swap(a18, a19, 0);

  zfp_swap(a19, a1b, 1);
  zfp_swap(a18, a1a, 1);

  zfp_swap(a1b, a1f, 2);
  zfp_swap(a1a, a1e, 2);
  zfp_swap(a19, a1d, 2);
  zfp_swap(a18, a1c, 2);
  /* 18, 19 */

  //encode_bit_plane(a17);
  //encode_bit_plane(a16);

  zfp_swap(a16, a17, 0);
  /* 16, 17 */

  //encode_bit_plane(a15);
  //encode_bit_plane(a14);

  zfp_swap(a14, a15, 0);

  zfp_swap(a15, a17, 1);
  zfp_swap(a14, a16, 1);
  /* 14, 15 */

  //encode_bit_plane(a13);
  //encode_bit_plane(a12);

  zfp_swap(a12, a13, 0);
  /* 12, 13 */

  //encode_bit_plane(a11);
  //encode_bit_plane(a10);

  zfp_swap(a10, a11, 0);

  zfp_swap(a11, a13, 1);
  zfp_swap(a10, a12, 1);

  zfp_swap(a13, a17, 2);
  zfp_swap(a12, a16, 2);
  zfp_swap(a11, a15, 2);
  zfp_swap(a10, a14, 2);

  zfp_swap(a17, a1f, 3);
  zfp_swap(a16, a1e, 3);
  zfp_swap(a15, a1d, 3);
  zfp_swap(a14, a1c, 3);
  zfp_swap(a13, a1b, 3);
  zfp_swap(a12, a1a, 3);
  zfp_swap(a11, a19, 3);
  zfp_swap(a10, a18, 3);
  /* 10, 11 */

  //encode_bit_plane(a0f);
  //encode_bit_plane(a0e);

  zfp_swap(a0e, a0f, 0);
  /* 0e, 0f */

  //encode_bit_plane(a0d);
  //encode_bit_plane(a0c);

  zfp_swap(a0c, a0d, 0);

  zfp_swap(a0d, a0f, 1);
  zfp_swap(a0c, a0e, 1);
  /* 0c, 0d */

  //encode_bit_plane(a0b);
  //encode_bit_plane(a0a);

  zfp_swap(a0a, a0b, 0);
  /* 0a, 0b */

  //encode_bit_plane(a09);
  //encode_bit_plane(a08);

  zfp_swap(a08, a09, 0);

  zfp_swap(a09, a0b, 1);
  zfp_swap(a08, a0a, 1);

  zfp_swap(a0b, a0f, 2);
  zfp_swap(a0a, a0e, 2);
  zfp_swap(a09, a0d, 2);
  zfp_swap(a08, a0c, 2);
  /* 08, 09 */

  //encode_bit_plane(a07);
  //encode_bit_plane(a06);

  zfp_swap(a06, a07, 0);
  /* 06, 07 */

  //encode_bit_plane(a05);
  //encode_bit_plane(a04);

  zfp_swap(a04, a05, 0);

  zfp_swap(a05, a07, 1);
  zfp_swap(a04, a06, 1);
  /* 04, 05 */

  //encode_bit_plane(a03);
  //encode_bit_plane(a02);

  zfp_swap(a02, a03, 0);
  /* 02, 03 */

  //encode_bit_plane(a01);
  //encode_bit_plane(a00);

  zfp_swap(a00, a01, 0);

  zfp_swap(a01, a03, 1);
  zfp_swap(a00, a02, 1);

  zfp_swap(a03, a07, 2);
  zfp_swap(a02, a06, 2);
  zfp_swap(a01, a05, 2);
  zfp_swap(a00, a04, 2);

  zfp_swap(a07, a0f, 3);
  zfp_swap(a06, a0e, 3);
  zfp_swap(a05, a0d, 3);
  zfp_swap(a04, a0c, 3);
  zfp_swap(a03, a0b, 3);
  zfp_swap(a02, a0a, 3);
  zfp_swap(a01, a09, 3);
  zfp_swap(a00, a08, 3);

  zfp_swap(a0f, a1f, 4);
  zfp_swap(a0e, a1e, 4);
  zfp_swap(a0d, a1d, 4);
  zfp_swap(a0c, a1c, 4);
  zfp_swap(a0b, a1b, 4);
  zfp_swap(a0a, a1a, 4);
  zfp_swap(a09, a19, 4);
  zfp_swap(a08, a18, 4);
  zfp_swap(a07, a17, 4);
  zfp_swap(a06, a16, 4);
  zfp_swap(a05, a15, 4);
  zfp_swap(a04, a14, 4);
  zfp_swap(a03, a13, 4);
  zfp_swap(a02, a12, 4);
  zfp_swap(a01, a11, 4);
  zfp_swap(a00, a10, 4);

  zfp_swap(a1f, a3f, 5);
  zfp_swap(a1e, a3e, 5);
  zfp_swap(a1d, a3d, 5);
  zfp_swap(a1c, a3c, 5);
  zfp_swap(a1b, a3b, 5);
  zfp_swap(a1a, a3a, 5);
  zfp_swap(a19, a39, 5);
  zfp_swap(a18, a38, 5);
  zfp_swap(a17, a37, 5);
  zfp_swap(a16, a36, 5);
  zfp_swap(a15, a35, 5);
  zfp_swap(a14, a34, 5);
  zfp_swap(a13, a33, 5);
  zfp_swap(a12, a32, 5);
  zfp_swap(a11, a31, 5);
  zfp_swap(a10, a30, 5);
  zfp_swap(a0f, a2f, 5);
  zfp_swap(a0e, a2e, 5);
  zfp_swap(a0d, a2d, 5);
  zfp_swap(a0c, a2c, 5);
  zfp_swap(a0b, a2b, 5);
  zfp_swap(a0a, a2a, 5);
  zfp_swap(a09, a29, 5);
  zfp_swap(a08, a28, 5);
  zfp_swap(a07, a27, 5);
  zfp_swap(a06, a26, 5);
  zfp_swap(a05, a25, 5);
  zfp_swap(a04, a24, 5);
  zfp_swap(a03, a23, 5);
  zfp_swap(a02, a22, 5);
  zfp_swap(a01, a21, 5);
  zfp_swap(a00, a20, 5);

  /* copy 64x64 matrix from input */
  a00 = data[0x3f - 0x00]; a01 = data[0x3f - 0x01]; a02 = data[0x3f - 0x02]; a03 = data[0x3f - 0x03];
  a04 = data[0x3f - 0x04]; a05 = data[0x3f - 0x05]; a06 = data[0x3f - 0x06]; a07 = data[0x3f - 0x07];
  a08 = data[0x3f - 0x08]; a09 = data[0x3f - 0x09]; a0a = data[0x3f - 0x0a]; a0b = data[0x3f - 0x0b];
  a0c = data[0x3f - 0x0c]; a0d = data[0x3f - 0x0d]; a0e = data[0x3f - 0x0e]; a0f = data[0x3f - 0x0f];
  a10 = data[0x3f - 0x10]; a11 = data[0x3f - 0x11]; a12 = data[0x3f - 0x12]; a13 = data[0x3f - 0x13];
  a14 = data[0x3f - 0x14]; a15 = data[0x3f - 0x15]; a16 = data[0x3f - 0x16]; a17 = data[0x3f - 0x17];
  a18 = data[0x3f - 0x18]; a19 = data[0x3f - 0x19]; a1a = data[0x3f - 0x1a]; a1b = data[0x3f - 0x1b];
  a1c = data[0x3f - 0x1c]; a1d = data[0x3f - 0x1d]; a1e = data[0x3f - 0x1e]; a1f = data[0x3f - 0x1f];
  a20 = data[0x3f - 0x20]; a21 = data[0x3f - 0x21]; a22 = data[0x3f - 0x22]; a23 = data[0x3f - 0x23];
  a24 = data[0x3f - 0x24]; a25 = data[0x3f - 0x25]; a26 = data[0x3f - 0x26]; a27 = data[0x3f - 0x27];
  a28 = data[0x3f - 0x28]; a29 = data[0x3f - 0x29]; a2a = data[0x3f - 0x2a]; a2b = data[0x3f - 0x2b];
  a2c = data[0x3f - 0x2c]; a2d = data[0x3f - 0x2d]; a2e = data[0x3f - 0x2e]; a2f = data[0x3f - 0x2f];
  a30 = data[0x3f - 0x30]; a31 = data[0x3f - 0x31]; a32 = data[0x3f - 0x32]; a33 = data[0x3f - 0x33];
  a34 = data[0x3f - 0x34]; a35 = data[0x3f - 0x35]; a36 = data[0x3f - 0x36]; a37 = data[0x3f - 0x37];
  a38 = data[0x3f - 0x38]; a39 = data[0x3f - 0x39]; a3a = data[0x3f - 0x3a]; a3b = data[0x3f - 0x3b];
  a3c = data[0x3f - 0x3c]; a3d = data[0x3f - 0x3d]; a3e = data[0x3f - 0x3e]; a3f = data[0x3f - 0x3f];
}

mg_TII(t, D, K) void
Decode2(t* Block, int B, i64 S, i8& N, bitstream* Bs) {
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  //std::cout << "Counter " << MyCounter << "P = " << int(P) << " X = " << X << " N = " << int(N) << std::endl;
  bool ExpectGroupTestBit = true;
  while (N < NVals) {
    Refill(Bs);
    int MaxBits = 64 - Bs->BitPos;
    u64 Next = Peek(Bs, MaxBits);
    i8 BitCurr = -1 + !ExpectGroupTestBit; i8 BitPrev = BitCurr;
    if (ExpectGroupTestBit) {
      BitCurr = Lsb(Next, -1);
      if (BitCurr != 0) {
        // there are no 1 bits, or the first bit is not 1 (the group is insignificant)
        Consume(Bs, 1);
        break;
      }
      /* group test bit is 1, at position 0. now move on to the next value 1-bit */
      mg_Assert(BitCurr == 0);
      //if ((BitCurr = TzCnt(Next = UnsetBit(Next, 0), MaxBits)) != MaxBits) // could be MaxBits
      BitCurr = Lsb(Next = UnsetBit(Next, 0), MaxBits);
      BitPrev = 0;
      if (BitCurr - BitPrev >= NVals - N)
        goto JUMP2;
    } else {
      goto JUMP;
    }
    /* BitCurr == position of the next value 1-bit (could be -1) */
    while (true) { // we loop through every other 1-bit
      N += BitCurr - BitPrev;
      if (BitCurr < MaxBits || N == NVals) {
        X += 1ull << (N - 1);
        //Block[N - 1] += 1ull << (N - 1);
      }
      if (BitCurr + 1 >= MaxBits) { // there is no bit left
        ExpectGroupTestBit = BitCurr < MaxBits;
        Consume(Bs, MaxBits);
        goto OUTER;
      } else if (!BitSet(Next, BitCurr + 1)) { // next group test bit is 0
        Consume(Bs, BitCurr + 2);
        goto DONE;
      } else { // next group test bit is 1
        Next &= ~(3ull << (BitCurr + 1)) >> 1; // unset BitCurr and BitCurr + 1
        BitPrev = BitCurr + 1;
JUMP:
        BitCurr = Lsb(Next, MaxBits);
        if (BitCurr - BitPrev >= NVals - N) {
JUMP2:
          Consume(Bs, (NVals - N) + BitPrev);
          N = NVals;
          X += 1ull << (NVals - 1);
          //Block[NVals - 1] += 1ull << (NVals - 1);
          goto DONE;
        }
      }
    }
OUTER:;
  }
DONE:
  //std::cout << "N = " << int(N) << std::endl;
  ++MyCounter;

  /* deposit bit plane from x */
  //for (int I = 0; X; ++I, X >>= 1)
  //  Block[I] += (t)(X & 1u) << B;
  __m256i Add = _mm256_set1_epi32(t(1) << B); // TODO: should be epi64 with t == u64
  __m256i Mask = _mm256_set_epi32(0xffffff7f, 0xffffffbf, 0xffffffdf, 0xffffffef, 0xfffffff7, 0xfffffffb, 0xfffffffd, 0xfffffffe);
  while (X) {
    __m256i Val = _mm256_set1_epi32(X);
    Val = _mm256_or_si256(Val, Mask);
    Val = _mm256_cmpeq_epi32(Val, _mm256_set1_epi64x(-1));
    //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
    _mm256_maskstore_epi32((int*)Block, Val, _mm256_add_epi32(_mm256_maskload_epi32((int*)Block, Val), Add));
    X >>= 8;
    Block += 8;
  }
}

mg_TII(t, D, K) void
Decode3(t* Block, int B, i64 S, i8& N, bitstream* Bs) {
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  //std::cout << "Counter " << MyCounter << "P = " << int(P) << " X = " << X << " N = " << int(N) << std::endl;
  bool ExpectGroupTestBit = true;
  while (N < NVals) {
    Refill(Bs);
    int MaxBits = 64 - Bs->BitPos;
    u64 Next = Peek(Bs, MaxBits);
    i8 BitCurr = -1 + !ExpectGroupTestBit; i8 BitPrev = BitCurr;
    if (ExpectGroupTestBit) {
      BitCurr = Lsb(Next, -1);
      if (BitCurr != 0) {
        // there are no 1 bits, or the first bit is not 1 (the group is insignificant)
        Consume(Bs, 1);
        break;
      }
      /* group test bit is 1, at position 0. now move on to the next value 1-bit */
      mg_Assert(BitCurr == 0);
      //if ((BitCurr = TzCnt(Next = UnsetBit(Next, 0), MaxBits)) != MaxBits) // could be MaxBits
      BitCurr = Lsb(Next = UnsetBit(Next, 0), MaxBits);
      BitPrev = 0;
      if (BitCurr - BitPrev >= NVals - N)
        goto JUMP2;
    } else {
      goto JUMP;
    }
    /* BitCurr == position of the next value 1-bit (could be -1) */
    while (true) { // we loop through every other 1-bit
      N += BitCurr - BitPrev;
      if (BitCurr < MaxBits || N == NVals) {
        X += 1ull << (N - 1);
        //Block[N - 1] += 1ull << (N - 1);
      }
      if (BitCurr + 1 >= MaxBits) { // there is no bit left
        ExpectGroupTestBit = BitCurr < MaxBits;
        Consume(Bs, MaxBits);
        goto OUTER;
      } else if (!BitSet(Next, BitCurr + 1)) { // next group test bit is 0
        Consume(Bs, BitCurr + 2);
        goto DONE;
      } else { // next group test bit is 1
        Next &= ~(3ull << (BitCurr + 1)) >> 1; // unset BitCurr and BitCurr + 1
        BitPrev = BitCurr + 1;
JUMP:
        BitCurr = Lsb(Next, MaxBits);
        if (BitCurr - BitPrev >= NVals - N) {
JUMP2:
          Consume(Bs, (NVals - N) + BitPrev);
          N = NVals;
          X += 1ull << (NVals - 1);
          //Block[NVals - 1] += 1ull << (NVals - 1);
          goto DONE;
        }
      }
    }
OUTER:;
  }
DONE:
  //std::cout << "N = " << int(N) << std::endl;
  ++MyCounter;

  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (t)(X & 1u) << B;
  //__m256i Add = _mm256_set1_epi32(t(1) << B); // TODO: should be epi64 with t == u64
  //__m256i Mask = _mm256_set_epi32(0xffffff7f, 0xffffffbf, 0xffffffdf, 0xffffffef, 0xfffffff7, 0xfffffffb, 0xfffffffd, 0xfffffffe);
  //while (X) {
  //  __m256i Val = _mm256_set1_epi32(X);
  //  Val = _mm256_or_si256(Val, Mask);
  //  Val = _mm256_cmpeq_epi32(Val, _mm256_set1_epi64x(-1));
  //  //int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
  //  _mm256_maskstore_epi32((int*)Block, Val, _mm256_add_epi32(_mm256_maskload_epi32((int*)Block, Val), Add));
  //  X >>= 8;
  //  Block += 8;
  //}
}

} // namespace mg

