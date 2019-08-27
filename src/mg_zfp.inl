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

template <typename t> void
PadBlock(t* P, int Nx, int Ny, int Nz) {
  for (int Z = 0; Z < 4; ++Z)
    for (int Y = 0; Y < 4; ++Y)
      PadBlock1D(P + Z * 16 + Y * 4, Nx, 1);

  for (int Z = 0; Z < 4; ++Z)
    for (int X = 0; X < 4; ++X)
      PadBlock1D(P + Z * 16 + X * 1, Ny, 4);

  for (int Y = 0; Y < 4; ++Y)
    for (int X = 0; X < 4; ++X)
      PadBlock1D(P + Y * 4 + X * 1, Nz, 16);
}

template <typename t> void
PadBlock2D(t* P, int Nx, int Ny) {
  for (int Y = 0; Y < 4; ++Y)
    PadBlock1D(P + Y * 4, Nx, 1);

  for (int X = 0; X < 4; ++X)
    PadBlock1D(P + X * 1, Ny, 4);
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
  if (MyCounter == 8679) {
    int Stop = 0;
  }
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  std::cout << "X = " << X << std::endl;
  for (; BitSize(*Bs) < S && N < NVals;) {
    if (Read(Bs)) {
      for (; BitSize(*Bs) < S && N + 1 < NVals;) {
        if (Read(Bs)) {
          break;
        } else {
          ++N;
        }
      }
      if (BitSize(*Bs) >= S)
        break;
      X += 1ull << (N++);
    } else {
      break;
    }
  }
  //std::cout << "N = " << int(N) << std::endl;
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (t)(X & 1u) << B;
  ++MyCounter;
}


mg_TII(t, D, K) void
Decode2(t* Block, int B, i64 S, i8& N, bitstream* Bs) {
  if (MyCounter == 8679) {
    int Stop = 0;
  }
  static_assert(is_unsigned<t>::Value);
  int NVals = pow<int, K>::Table[D];
  mg_Assert(NVals <= 64); // e.g. 4x4x4, 4x4, 8x8
  i8 P = (i8)Min((i64)N, S - BitSize(*Bs));
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  std::cout << "X = " << X << std::endl;
  bool ExpectGroupTestBit = true;
  while (N < NVals) {
    // NOTE: if there is one bit left in the bit plane, and the last group test
    // bit is 1, this last bit is inferred as 1 and not written
    Refill(Bs);
    int MaxBits = 64 - Bs->BitPos;
    u64 Next = Peek(Bs, MaxBits); // only the last MaxBits of Next are meaningful
    int Out[65] = { -1, 0 }; // TODO: aligned allocation?
    int M = DecodeBitmap(Next, Out + 1); // TODO: do we need to fill Out with 0s?
    Out[M + 1] = MaxBits; // sentinel
    if (ExpectGroupTestBit && (M == 0 || Out[1] > 0)) { 
      // there are no 1 bits, or the first bit is not 1 (the group is insignificant)
      Consume(Bs, 1);
      break;
    }
    /* INVARIANT: there is at least one 1-bit and if we are expecting a group test
    bit, it is 1 and is at the first position */
    if (ExpectGroupTestBit && M == 1) { // only see one group test bit (case 6)
      mg_Assert(Out[1] == 0);
      if (N + 1 == NVals) { // the group test bit is for the last significant coefficient
        Consume(Bs, 1);
        N = NVals;
        break;
      }
      // else, the group test bit is the only 1 bit
      Consume(Bs, MaxBits); // consume everything
      N += MaxBits - 1;
      mg_Assert(N < NVals);
      ExpectGroupTestBit = false;
      continue;
    }
    /* INVARIANT: there is at least one value 1-bit */
    int L = 0; // number of value bits to consume
    /* process the value bits */
    int I = ExpectGroupTestBit + 1; // start from either 1 (value) or 2 (group test)
    while (I - 1 < M) { // we loop through every other 1-bit
      L += Out[I] - Out[I - 1]; // assuming Out[0] is -1
      if (L + N >= NVals) { // we went pass the last group test bit (1)
        // TODO: double check the case where N + L == NVals and the last group test bit is 1 (for the last coefficient)
        // this cannot happen unless the last group test 1-bit is the last significant coefficient
        L -= Out[I] - Out[I - 1];
        mg_Assert(I >= 2);
        I -= 2; // go back to the last value bit (NOTE: I could go back to 0)
        break;
      }
      X += 1ull << (L - 1 + N);
      if ((I == M) || (Out[I + 1] > Out[I] + 1)) // there is no next 1-bit, or we found a 0 group test bit, the rest of the bit plane is insignificant
        break;
      I += 2;
    }
    // INVARIANT: Out[I] is the position of a value 1-bit (could be the last)
    // 0 <= Out[I] < MaxBits
    mg_Assert((I == 0) || (0 <= Out[I] && Out[I] < MaxBits));
    N += L;
    if (Out[I] + 1 < MaxBits) { // there is one (group test) bit following this
      bool NextBit = BitSet(Next, Out[I] + 1);
      if (!NextBit || (N + 1 == NVals)) {
        Consume(Bs, Out[I] + 2);
        N += NextBit;
        break;
      } else { // group test bit is 1
        mg_Assert(Out[I + 1] == Out[I] + 1);
        N += MaxBits - Out[I + 1];
        if (N >= NVals) {
          N -= MaxBits - Out[I + 1];
          Consume(Bs, Out[I + 1] + (NVals - N));
          N = NVals;
          break;
        } else {
          Consume(Bs, MaxBits);
          ExpectGroupTestBit = false;
          continue;
        }
      }
    } else { // there is no bit following it
      ExpectGroupTestBit = true;
      Consume(Bs, Out[I] + 1);
      continue;
    }
  }
  //std::cout << "N = " << int(N) << std::endl;
  ++MyCounter;

  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (t)(X & 1u) << B;
}

} // namespace mg

