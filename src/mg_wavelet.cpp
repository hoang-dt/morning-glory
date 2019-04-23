#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common_types.h"
#include "mg_math.h"
#include "mg_types.h"
#include "mg_volume.h"
#include "mg_wavelet.h"

namespace mg {

// TODO: this won't work for a general (sub)volume
void Cdf53Forward(volume* Vol, int NLevels) {
#define Body(type)\
  v3i Dims = Extract3Ints64(Vol->DimsCompact);\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  for (int I = 0; I < NLevels; ++I) {\
    ForwardLiftCdf53X(FPtr, Dims, v3i(I, I, I));\
    ForwardLiftCdf53Y(FPtr, Dims, v3i(I, I, I));\
    ForwardLiftCdf53Z(FPtr, Dims, v3i(I, I, I));\
  }\

  TypeChooser(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void Cdf53Inverse(volume* Vol, int NLevels) {
#define Body(type)\
  v3i Dims = Extract3Ints64(Vol->DimsCompact);\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  for (int I = NLevels - 1; I >= 0; --I) {\
    InverseLiftCdf53Z(FPtr, Dims, v3i(I, I, I));\
    InverseLiftCdf53Y(FPtr, Dims, v3i(I, I, I));\
    InverseLiftCdf53X(FPtr, Dims, v3i(I, I, I));\
  }\

  TypeChooser(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void Cdf53ForwardExtrapolate(volume* Vol) {
#define Body(type)\
  v3i SmallDims = Extract3Ints64(Vol->Extent.DimsCompact);\
  v3i BigDims = Extract3Ints64(Vol->DimsCompact);\
  if (BigDims.Y > 1)\
    mg_Assert(BigDims.X == BigDims.Y);\
  if (BigDims.Z > 1)\
    mg_Assert(BigDims.Y == BigDims.Z);\
  mg_Assert(IsPow2(BigDims.X - 1));\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  int NLevels = Log2Floor(BigDims.X - 1) + 1;\
  for (int I = 0; I < NLevels; ++I) {\
    ForwardLiftExtrapolateCdf53X(FPtr, SmallDims, BigDims, v3i(I, I, I));\
    ForwardLiftExtrapolateCdf53Y(FPtr, SmallDims, BigDims, v3i(I, I, I));\
    ForwardLiftExtrapolateCdf53Z(FPtr, SmallDims, BigDims, v3i(I, I, I));\
  }\

  TypeChooser(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void Cdf53InverseExtrapolate(volume* Vol) {
#define Body(type)\
  v3i SmallDims = Extract3Ints64(Vol->Extent.DimsCompact);\
  v3i BigDims = Extract3Ints64(Vol->DimsCompact);\
  if (BigDims.Y > 1)\
    mg_Assert(BigDims.X == BigDims.Y);\
  if (BigDims.Z > 1)\
    mg_Assert(BigDims.Y == BigDims.Z);\
  mg_Assert(IsPow2(BigDims.X - 1));\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  int NLevels = Log2Floor(BigDims.X - 1) + 1;\
  for (int I = NLevels - 1; I >= 0; --I) {\
    InverseLiftExtrapolateCdf53Z(FPtr, SmallDims, BigDims, v3i(I, I, I));\
    InverseLiftExtrapolateCdf53Y(FPtr, SmallDims, BigDims, v3i(I, I, I));\
    InverseLiftExtrapolateCdf53X(FPtr, SmallDims, BigDims, v3i(I, I, I));\
  }\

  TypeChooser(Vol->Type)
#undef Body
}

array<u8, 8> SubbandOrders[4] = {
  { 127, 127, 127, 127, 127, 127, 127, 127 }, // not used
  { 0, 1, 127, 127, 127, 127, 127, 127 }, // for 1D
  { 0, 1, 2, 3, 127, 127, 127, 127 }, // for 2D
  { 0, 1, 2, 4, 3, 5, 6, 7 } // for 3D
};

/* Here we assume the wavelet transform is done in X, then Y, then Z */
void BuildSubbands(v3i N, int NLevels, dynamic_array<extent>* Subbands) {
  int NDims = NumDims(N);
  const array<u8, 8>& Order = SubbandOrders[NDims];
  Reserve(Subbands, ((1 << NDims) - 1) * NLevels + 1);
  v3i M = N;
  for (int I = 0; I < NLevels; ++I) {
    v3i P((M.X + 1) >> 1, (M.Y + 1) >> 1, (M.Z + 1) >> 1);
    for (int J = (1 << NDims) - 1; J > 0; --J) {
      u8 Z = Order[J] & 1u,
         Y = (Order[J] >> (NDims - 2)) & 1u,
         X = (Order[J] >> (NDims - 1)) & 1u;
      v3i Sm((X == 0) ? P.X : M.X - P.X,
             (Y == 0) ? P.Y : M.Y - P.Y,
             (Z == 0) ? P.Z : M.Z - P.Z);
      if (NDims == 3 && Sm.X != 0 && Sm.Y != 0 && Sm.Z != 0) // child exists
        PushBack(Subbands, extent(v3i(X, Y, Z) * P, Sm));
      else if (NDims == 2 && Sm.X != 0 && Sm.Y != 0)
        PushBack(Subbands, extent(v3i(X * P.X, Y * P.Y, 0), v3i(Sm.X, Sm.Y, 1)));
    }
    M = P;
  }
  if (NDims == 2) mg_Assert(M.Z == 1);
  PushBack(Subbands, extent(v3i(0, 0, 0), M));
  Reverse(Begin(*Subbands), End(*Subbands));
}

/* Here we assume the wavelet transform is done in X, then Y, then Z */
int LevelToSubband(v3i Level) {
  if (Level.X + Level.Y + Level.Z == 0)
    return 0;
  static constexpr int8_t Table[] = { 0, 1, 2, 4, 3, 5, 6, 7 };
  int Lvl = Max(Max(Level.X, Level.Y), Level.Z);
  return 7 * (Lvl - 1) + Table[4*(Level.X == Lvl) + 2 * (Level.Y == Lvl) + 1 * (Level.Z == Lvl)];
}

} // namespace mg

