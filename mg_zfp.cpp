#include "mg_algorithm.h"
#include "mg_bitstream.h"
#include "mg_zfp.h"

namespace mg {

const v3i ZDims(4, 4, 4);

/* Only return true if the block is fully encoded */
bool EncodeBlock(const u64* Block, int Bitplane, int BitsMax, i8& N, i8& M,
                 bool& InnerLoop, bitstream* Bs)
{
  /* extract bit plane Bitplane to X */
  mg_Assert(N <= 64);
  u64 X = 0;
  for (int I = M; I < 64; ++I)
    X += u64((Block[I] >> Bitplane) & 1u) << I;
  i8 P = Min(N - M, BitsMax - (int)BitSize(*Bs));
  WriteLong(Bs, X, P);
  X >>= P; // P == 64 is fine since in that case we don't need X any more
  u64 LastBit = 0;
  if (InnerLoop) goto INNER_LOOP;
  InnerLoop = false;
  // TODO: we may be able to speed this up by getting rid of the shift of X
  for (; BitSize(*Bs) < BitsMax && N < 64 && (LastBit = Write(Bs, !!X)); X >>= 1, ++N) {
 INNER_LOOP:
    InnerLoop = true;
    for (; BitSize(*Bs) < BitsMax && N < 64 - 1 && !Write(Bs, X & 1u); X >>= 1, ++N);
    InnerLoop = false;
  }
  mg_Assert(N <= 64);
  M += P;
  return (N == 64 || LastBit == 0);
}

/* Only return true if the block is fully decoded */
bool DecodeBlock(u64* Block, int Bitplane, int BitsMax, i8& N, i8& M,
                 bool& InnerLoop, bitstream* Bs)
{
  i8 P = Min(N - M, BitsMax - (int)BitSize(*Bs));
  /* decode first N bits of bit plane #Bitplane */
  u64 X = ReadLong(Bs, P);
  /* unary run-length decode remainder of bit plane */
  u64 LastBit = 0;
  i8 D = 0;
  if (InnerLoop) goto INNER_LOOP;
  InnerLoop = false;
  for (; BitSize(*Bs) < BitsMax && (N + D) < 64 && (LastBit = Read(Bs)); 
       X += 1ull << (P + D++)) {
 INNER_LOOP:
    InnerLoop = true;
    for (; BitSize(*Bs) < BitsMax && (N + D) < 64 - 1 && !Read(Bs); ++D);
    InnerLoop = false;
  }
  /* deposit bit plane from x */
  for (int I = M; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
  M += P;
  N += D;
  return (N == 64 || LastBit == 0);
}

} // namespace mg
