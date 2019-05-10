#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_common.h"
#include "mg_volume.h"
#include "mg_wavelet.h"

namespace mg {

// TODO: this won't work for a general (sub)volume
void
ForwardCdf53(volume* Vol, int NLevels) {
#define Body(type)\
  v3i Dims3 = Dims(*Vol);\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  for (int I = 0; I < NLevels; ++I) {\
    FLiftCdf53X(FPtr, Dims3, v3i(I));\
    FLiftCdf53Y(FPtr, Dims3, v3i(I));\
    FLiftCdf53Z(FPtr, Dims3, v3i(I));\
  }\

  mg_DispatchOnType(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void
InverseCdf53(volume* Vol, int NLevels) {
#define Body(type)\
  v3i Dims3 = Dims(*Vol);\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  for (int I = NLevels - 1; I >= 0; --I) {\
    ILiftCdf53Z(FPtr, Dims3, v3i(I));\
    ILiftCdf53Y(FPtr, Dims3, v3i(I));\
    ILiftCdf53X(FPtr, Dims3, v3i(I));\
  }\

  mg_DispatchOnType(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void
ForwardCdf53Ext(grid_volume* Grid) {
#define Body(type)\
  v3i N = Dims(*Grid);\
  v3i NN = Dims(Grid->Base);\
  if (NN.Y > 1) mg_Assert(NN.X == NN.Y);\
  if (NN.Z > 1) mg_Assert(NN.Y == NN.Z);\
  mg_Assert(IsPow2(NN.X - 1));\
  type* FPtr = (type*)(Grid->Base.Buffer.Data);\
  int NLevels = Log2Floor(NN.X - 1) + 1;\
  for (int I = 0; I < NLevels; ++I) {\
    FLiftExtCdf53X(FPtr, N, NN, v3i(I));\
    FLiftExtCdf53Y(FPtr, N, NN, v3i(I));\
    FLiftExtCdf53Z(FPtr, N, NN, v3i(I));\
  }\

  mg_DispatchOnType(Grid->Base.Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void
InverseCdf53Ext(grid_volume* Grid) {
#define Body(type)\
  v3i N = Dims(*Grid);\
  v3i NN = Dims(Grid->Base);\
  if (NN.Y > 1) mg_Assert(NN.X == NN.Y);\
  if (NN.Z > 1) mg_Assert(NN.Y == NN.Z);\
  mg_Assert(IsPow2(NN.X - 1));\
  type* FPtr = (type*)(Grid->Base.Buffer.Data);\
  int NLevels = Log2Floor(NN.X - 1) + 1;\
  for (int I = NLevels - 1; I >= 0; --I) {\
    ILiftExtCdf53Z(FPtr, N, NN, v3i(I));\
    ILiftExtCdf53Y(FPtr, N, NN, v3i(I));\
    ILiftExtCdf53X(FPtr, N, NN, v3i(I));\
  }\

  mg_DispatchOnType(Grid->Base.Type)
#undef Body
}

stack_array<u8, 8>
SubbandOrders[4] = {
  { 127, 127, 127, 127, 127, 127, 127, 127 }, // not used
  { 0, 1, 127, 127, 127, 127, 127, 127 }, // for 1D
  { 0, 1, 2, 3, 127, 127, 127, 127 }, // for 2D
  { 0, 1, 2, 4, 3, 5, 6, 7 } // for 3D
};

/* Here we assume the wavelet transform is done in X, then Y, then Z */
void
BuildSubbands(const v3i& N, int NLevels, array<grid>* Subbands) {
  int NDims = NumDims(N);
  stack_array<u8, 8>& Order = SubbandOrders[NDims];
  Reserve(Subbands, ((1 << NDims) - 1) * NLevels + 1);
  v3i M = N;
  for (int I = 0; I < NLevels; ++I) {
    v3i P((M.X + 1) >> 1, (M.Y + 1) >> 1, (M.Z + 1) >> 1);
    for (int J = (1 << NDims) - 1; J > 0; --J) {
      u8 Z = BitSet(Order[J], Max(NDims - 3, 0)),
         Y = BitSet(Order[J], Max(NDims - 2, 0)),
         X = BitSet(Order[J], Max(NDims - 1, 0));
      v3i Sm((X == 0) ? P.X : M.X - P.X,
             (Y == 0) ? P.Y : M.Y - P.Y,
             (Z == 0) ? P.Z : M.Z - P.Z);
      if (NDims == 3 && Sm.X != 0 && Sm.Y != 0 && Sm.Z != 0) // child exists
        PushBack(Subbands, grid(v3i(X, Y, Z) * P, Sm));
      else if (NDims == 2 && Sm.X != 0 && Sm.Y != 0)
        PushBack(Subbands, grid(v3i(X * P.X, Y * P.Y, 0), v3i(Sm.X, Sm.Y, 1)));
      else
        PushBack(Subbands, grid(v3i(X * P.X, 0, 0), v3i(Sm.X, 1, 1)));
    }
    M = P;
  }
  PushBack(Subbands, grid(v3i(0), M));
  Reverse(Begin(*Subbands), End(*Subbands));
}

/* This version assumes the coefficients were not moved */
void
BuildSubbandsInPlace(const v3i& N, int NLevels, array<grid>* Subbands) {
  int NDims = NumDims(N);
  stack_array<u8, 8>& Order = SubbandOrders[NDims];
  Reserve(Subbands, ((1 << NDims) - 1) * NLevels + 1);
  v3i M = N; // dimensions of all the subbands at the current level
  v3i S(1, 1, 1); // strides
  for (int I = 0; I < NLevels; ++I) {
    S = S * 2;
    v3i P((M.X + 1) >> 1, (M.Y + 1) >> 1, (M.Z + 1) >> 1); // next dimensions
    for (int J = (1 << NDims) - 1; J > 0; --J) { // for each subband
      u8 Z = BitSet(Order[J], Max(NDims - 3, 0)),
         Y = BitSet(Order[J], Max(NDims - 2, 0)),
         X = BitSet(Order[J], Max(NDims - 1, 0));
      v3i Sm((X == 0) ? P.X : M.X - P.X, // dimensions of the current subband
             (Y == 0) ? P.Y : M.Y - P.Y,
             (Z == 0) ? P.Z : M.Z - P.Z);
      if (NDims == 3 && Sm.X != 0 && Sm.Y != 0 && Sm.Z != 0) // subband exists
        PushBack(Subbands, grid(v3i(X, Y, Z) * S / 2, Sm, S));
      else if (NDims == 2 && Sm.X != 0 && Sm.Y != 0)
        PushBack(Subbands, grid(v3i(X, Y, 0) * S / 2, v3i(Sm.X, Sm.Y, 1), S));
      else if (NDims == 1 && Sm.X != 0)
        PushBack(Subbands, grid(v3i(X, 0, 0) * S / 2, v3i(Sm.X, 1, 1), S));
    }
    M = P;
  }
  PushBack(Subbands, grid(v3i(0), M, S)); // final (coarsest) subband
  Reverse(Begin(*Subbands), End(*Subbands));
}

int
LevelToSubband(const v3i& Level) {
  if (Level.X + Level.Y + Level.Z == 0)
    return 0;
  static constexpr i8 Table[] = { 0, 1, 2, 4, 3, 5, 6, 7 };
  int Lvl = Max(Max(Level.X, Level.Y), Level.Z);
  return 7 * (Lvl - 1) + Table[4 * (Level.X == Lvl) +
                               2 * (Level.Y == Lvl) +
                               1 * (Level.Z == Lvl)];
}

void
FormSubbands(grid_volume Dst, grid_volume Src, int NLevels) {
  mg_Assert(Dst.Grid.Dims == Src.Grid.Dims);
  mg_Assert(Dst.Base.Type == Src.Base.Type);
  v3i Dims3 = Dims(Src);
  array<grid> Subbands;
  array<grid> SubbandsInPlace;
  BuildSubbands(Dims3, NLevels, &Subbands);
  BuildSubbandsInPlace(Dims3, NLevels, &SubbandsInPlace);
  for (int I = 0; I < Size(Subbands); ++I) {
    Dst.Grid = Subbands[I];
    Src.Grid = SubbandsInPlace[I];
    Copy(Src, &Dst);
  }
}

//v3i SubbandToLevel(int S) {
//  if (S == 0)
//    return v3i(0, 0, 0);
//  /* handle level 0 which has only 1 subband (other levels have 7 subbands) */
//  int Lvl = (S + 6) / 7;
//  /* subtract all subbands on previous levels (except the subband 0);
//  basically it reduces the case to the 2x2x2 case where subband 0 is in corner */
//  S -= 7 * (Lvl - 1);
//  /* bit 0 -> x axis offset; bit 1 -> y axis offset; bit 2 -> z axis offset
//  we subtract from Lvl as it corresponds to the +x, +y, +z corner */
//  return v3i(Lvl - !bm::bit_test(i, 0), lvl - !bm::bit_test(i, 1), lvl - !bm::bit_test(i, 2));
//}

v3i
ExpandDomain(const v3i& N, int NLevels) {
  v3i Count(0, 0, 0);
  v3i M = N;
  for (int I = 0; I < NLevels; ++I) {
    v3i Add(IsEven(M.X), IsEven(M.Y), IsEven(M.Z));
    Count = Count + Add;
    M = (M + Add + 1) / 2;
  }
  return N + Count;
}

} // namespace mg


