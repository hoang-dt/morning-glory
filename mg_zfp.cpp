#include "mg_algorithm.h"
#include "mg_bitstream.h"
#include "mg_zfp.h"

namespace mg {

const v3i ZfpBlockDims(4, 4, 4);

/* Only return true if the block is fully encoded */
bool EncodeBlock(const u64* Block, int Bitplane, int BitsMax, i8& N, i8& M,
                 bool& InnerLoop, bitstream* Bs) {
  /* extract bit plane Bitplane to X */
  mg_Assert(N <= 64);
  u64 X = 0;
  for (int I = 0; I < 64; ++I)
    X += u64((Block[I] >> Bitplane) & 1u) << I;
  /* code the last N bits of bit plane b */
  M = Min(N - M, BitsMax - (int)BitSize(*Bs));
  mg_Assert(M >= 0);
  WriteLong(Bs, X, M);
  X >>= N;
  u64 LastBit = 0;
  if (InnerLoop) goto INNER_LOOP;
  InnerLoop = false;
  for (; BitSize(*Bs) < BitsMax && N < 64 && (LastBit = Write(Bs, !!X)); X >>= 1, ++N) {
 INNER_LOOP:
    InnerLoop = true;
    for (; BitSize(*Bs) < BitsMax && N < 64 - 1 && !Write(Bs, X & 1u); X >>= 1, ++N);
    InnerLoop = false;
  }
  mg_Assert(N <= 64);
  return (N == 64 || LastBit == 0);
}

/* Only return true if the block is fully decoded */
bool DecodeBlock(u64* Block, int Bitplane, int BitsMax, i8& N, i8& M,
                 bool& InnerLoop, bitstream* Bs) {
  M = Min(N - M, BitsMax - (int)BitSize(*Bs));
  /* decode first N bits of bit plane #Bitplane */
  u64 X = ReadLong(Bs, M);
  // TODO: what if M = 0?
  /* unary run-length decode remainder of bit plane */
  u64 LastBit = 0;
  if (InnerLoop) goto INNER_LOOP;
  InnerLoop = false;
  for (; BitSize(*Bs) < BitsMax && N < 64 && (LastBit = Read(Bs)); X += u64(1) << N++) {
 INNER_LOOP:
    InnerLoop = true;
    for (; BitSize(*Bs) < BitsMax && N < 64 - 1 && !Read(Bs); ++N);
    InnerLoop = false;
  }
  /* deposit bit plane from x */
  for (int I = 0; X; ++I, X >>= 1)
    Block[I] += (u64)(X & 1u) << Bitplane;
  return (N == 64 || LastBit == 0);
}

} // namespace mg
