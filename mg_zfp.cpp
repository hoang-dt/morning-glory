#include "mg_algorithm.h"
#include "mg_bitstream.h"
#include "mg_zfp.h"

namespace mg {

const v3i ZDims(4, 4, 4);

/* Only return true if the block is fully encoded */
bool EncodeBlock2(const u64* Block, int Bitplane, i8& N, bitstream* Bs)
{
  /* extract bit plane Bitplane to X */
  mg_Assert(N <= 64);
  u64 X = 0;
  for (int I = 0; I < 64; ++I)
    X += u64((Block[I] >> Bitplane) & 1u) << I;
  WriteLong(Bs, X, N);
  u64 Y = X;
  for (int I = 0; I < N; ++I) {
    //printf("%d", Y & 1);
    Y >>= 1;
  }
  X >>= N;
  //printf("0 %d %llu\n", N, X);
  //if (N > 0)
    //printf("0 %d %llu\n", N, (X & (~0ull >> (64 - N))));
  //else
    //printf("0 %d %llu\n", N, 0);
  // TODO: we may be able to speed this up by getting rid of the shift of X
  int Bit = 0;
  for (; N < 64 && (Bit = Write(Bs, !!X), /*printf("%d", Bit),*/ Bit); X >>= 1, ++N) {
    //printf("1 %d\n", Bit);
    for (; N < 64 - 1 && (!(Bit = Write(Bs, X & 1u)), /*printf("%d", Bit),*/ !Bit); X >>= 1, ++N);
    //printf("2 %d\n", Bit);
  }
  mg_Assert(N <= 64);
  return (N == 64);
}
bool DecodeBlock2(u64* Block, int Bitplane, i8& N, bitstream* Bs)
{
  /* decode first N bits of bit plane #Bitplane */
  u64 X = ReadLong(Bs, N);
  printf("0 %d %llu\n", N, X);
  /* unary run-length decode remainder of bit plane */
  int Bit = 0;
  for (; N < 64 && (Bit = Read(Bs)); X += 1ull << N++) {
    printf("1 %d\n", Bit);
    for (; N < 64 - 1 && !(Bit = Read(Bs)); ++N);
    printf("2 %d\n", Bit);
  }
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
  return (N == 64);
}
/* Only return true if the block is fully encoded */
bool EncodeBlock(const u64* Block, int Bitplane, int S, i8& N, i8& M,
                 bool& InnerLoop, bitstream* Bs)
{
  /* extract bit plane Bitplane to X */
  mg_Assert(N <= 64);
  u64 X = 0;
  for (int I = M; I < 64; ++I)
    X += u64((Block[I] >> Bitplane) & 1u) << (I - M);
  i8 P = Min(N - M, S - (int)BitSize(*Bs));
  if (P > 0) {
    WriteLong(Bs, X, P);
    X >>= P; // P == 64 is fine since in that case we don't need X any more
  }
  u64 Lb = 1;
  if (InnerLoop) goto INNER_LOOP;
  // TODO: we may be able to speed this up by getting rid of the shift of X
  // or the call bit BitSize()
  for (; BitSize(*Bs) < S && N < 64;) {
    if ((Lb = Write(Bs, !!X))) {
INNER_LOOP:
      for (; BitSize(*Bs) < S && N < 64 - 1;) {
        if (Write(Bs, X & 1u)) {
          break;
        } else {
          X >>= 1;
          ++N;
          ++P;
        }
      }
      if (BitSize(*Bs) >= S) {
        InnerLoop = true;
        break;
      }
      X >>= 1;
      ++N;
      ++P;
    } else {
      break;
    }
  }
  mg_Assert(N <= 64);
  M += P;
  // TODO: it seems M == N is always true?
  return ((N == 64 && M == N) || Lb == 0);
}

/* Only return true if the block is fully decoded */
bool DecodeBlock(u64* Block, int Bitplane, int S, i8& N, i8& M,
                 bool& InnerLoop, bitstream* Bs)
{
  i8 P = Min(N - M, S - (int)BitSize(*Bs));
  /* decode first N bits of bit plane #Bitplane */
  u64 X = P > 0 ? ReadLong(Bs, P) : 0;
  /* unary run-length decode remainder of bit plane */
  u64 Lb = 1;
  if (InnerLoop) goto INNER_LOOP;
  for (; BitSize(*Bs) < S && N < 64;) {
    if ((Lb = Read(Bs))) {
 INNER_LOOP:
      for (; BitSize(*Bs) < S && N < 64 - 1;) {
        if (Read(Bs)) {
          break;
        } else {
          ++N;
          ++P;
        }
      }
      if (BitSize(*Bs) >= S) {
        InnerLoop = true;
        break;
      }
      X += 1ull << (P++);
      ++N;
    } else {
      break;
    }
  }
  /* deposit bit plane from x */
  for (int I = M; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
  M += P;
  return ((N == 64 && M == N)|| Lb == 0);
}

} // namespace mg
