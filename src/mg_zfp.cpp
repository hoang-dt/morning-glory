#include "mg_algorithm.h"
#include "mg_bitstream.h"
#include "mg_zfp.h"

namespace mg {

const v3i ZDims(4, 4, 4);

/* Only return true if the block is fully encoded */
bool EncodeBlockOriginal(const u64* Block, int Bitplane, i8& N, bitstream* Bs) {
  /* extract bit plane Bitplane to X */
  mg_Assert(N <= 64);
  u64 X = 0;
  for (int I = 0; I < 64; ++I)
    X += u64((Block[I] >> Bitplane) & 1u) << I;
  WriteLong(Bs, X, N);
  X >>= N;
  // TODO: we may be able to speed this up by getting rid of the shift of X
  for (; N < 64 && Write(Bs, !!X); X >>= 1, ++N) {
    for (; N < 64 - 1 && !Write(Bs, X & 1u); X >>= 1, ++N);
  }
  mg_Assert(N <= 64);
  return (N == 64);
}

bool DecodeBlockOriginal(u64* Block, int Bitplane, i8& N, bitstream* Bs) {
  /* decode first N bits of bit plane #Bitplane */
  u64 X = ReadLong(Bs, N);
  /* unary run-length decode remainder of bit plane */
  for (; N < 64 && Read(Bs); X += 1ull << N++) {
    for (; N < 64 - 1 && !Read(Bs); ++N);
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
