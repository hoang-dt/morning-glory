#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_wavelet.h"

namespace mg {

// TODO: this won't work for a general (sub)volume
void
ForwardCdf53Old(volume* Vol, int NLevels) {
#define Body(type)\
  v3i Dims3 = Dims(*Vol);\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  for (int I = 0; I < NLevels; ++I) {\
    FLiftCdf53OldX(FPtr, Dims3, v3i(I));\
    FLiftCdf53OldY(FPtr, Dims3, v3i(I));\
    FLiftCdf53OldZ(FPtr, Dims3, v3i(I));\
  }\

  mg_DispatchOnType(Vol->Type)
#undef Body
}

//void
//ForwardCdf53Tile(
//  int Lvl, // level of the current tile
//  int Idx, // index of the current tile
//  bool Last, // acts as a "flush" signal when we reach the last tile
//  const v3i& TDims3, // dimensions of a tile (e.g. 32 x 32 x 32)
//  array<array<v3i>>* RDims3s, // real dimensions of the 8 tiles on each level
//  array<array<volume>>* Vols) // data of the 8 tiles on each level
//{
//  mg_Assert(IsEven(TDims3.X) && IsEven(TDims3.Y) && IsEven(TDims3.Z));
//  mg_Assert(Size(*Vols) > Lvl + 1);
//  const int NSbands = 8; // number of subbands in 3D
//  if (Lvl >= Size(*Vols))
//    return; // base case, end the recursive calls
//  /* transform the current tile */
//  int NIdx = Idx % NSbands; // normalized index
//  v3i& M = (*RDims3s)[Lvl][]; // dims of the current tile
//  if (!Last)
//    mg_Assert(IsEven(M.X) && IsEven(M.Y) && IsEven(M.Z));
//  FLiftCdf53X<f64>(grid(M), M, lift_option::NoUpdateLast, Vols[]);
//  M.X += IsEven(M.X);
//  FLiftCdf53Y<f64>(grid(M), M, lift_option::NoUpdateLast, Vol);
//  M.Y += IsEven(M.Y);
//  FLiftCdf53Z<f64>(grid(M), M, lift_option::NoUpdateLast, Vol);
//  M.Z += IsEven(M.Z);
//  // TODO: update the dims3 of the parent tile
//  /* initialize the memory for the parent tiles if this is the first children */
//  stack_linear_allocator<8> Alloc;
//  array<extent> Sbands(&Alloc);
//  BuildSubbands(M, 1, &Sbands);
//  // TODO: what about 2D?
//  int NLvl = Lvl + 1;
//  if (NIdx == 0) {
//    Resize(&(*Vols)[NLvl], NSbands);
//    Resize(&(*RDims3s)[NLvl], NSbands);
//  }
//  u32 NIdx = Idx % NSbands; // normalized index
//  v3i HDims3 = (TDims3 + 1) / 2; // half dims
//  v3i P((NIdx & 1) * HDims3.X, (NIdx & 2) * HDims3.Y, (NIdx & 4) * HDims3.Z);
//  /* spread the samples to the 8 parent subbands */
//  for (int Sb = 0; Sb < NSbands; ++Sb) {
//    if (NIdx == 0) { // first children, allocate memory
//      buffer Buf;
//      AllocBuf0(&Buf, sizeof(f64) * Prod(TDims3 + 1));
//      (*Vols)[NLvl][Sb] = volume(Buf, TDims3 + 1);
//      (*RDims3s)[NLvl][Sb] = v3i::Zero;
//    }
//    /* copy the samples */
//    Add(Sbands[Sb], (*Vols)[Lvl][NIdx], extent(P, Dims(Sbands[Sb])), (*Vols)[NLvl][Sb]);
//    /* update the dimensions */
//    v3i& PDims3 = (*RDims3s)[NLvl][Sb];
//    if (P.X == 0 && P.Y == 0 && P.Z == 0)
//      PDims3 += Dims(Sbands[Sb]);
//    else if (P.X > 0 && P.Y == 0 && P.Z == 0)
//      PDims3.X += Dims(Sbands[Sb]).X;
//    else if (P.X == 0 && P.Y > 0 && P.Z == 0)
//      PDims3.Y += Dims(Sbands[Sb]).Y;
//    else if (P.X == 0 && P.Y == 0 && P.X > 0)
//      PDims3.Z += Dims(Sbands[Sb]).Z;
//  }
//  ZeroBuf(&((*Vols)[Lvl][NIdx].Buffer)); // clear the current tile's buffer
//  /* if this is the last children, recurse */
//  if (Last || NIdx + 1 == NSbands)
//    ForwardCdf53Tile(NLvl, Idx / NSbands, Last, TDims3, RDims3s, Vols);
//}

//void
//ForwardCdf53Tile2D(
//  int Lvl, // level of the current tile
//  int Idx, // index of the current tile
//  bool Last, // acts as a "flush" signal when we reach the last tile
//  const v3i& TDims3, // dimensions of a tile (e.g. 32 x 32)
//  array<array<v3i>>* RDims3s, // real dimensions of the 4 tiles on each level
//  array<array<volume>>* Vols) // data of the 4 tiles on each level
//{
//  mg_Assert(IsEven(TDims3.X) && IsEven(TDims3.Y));
//  mg_Assert(Size(*Vols) > Lvl + 1);
//  const int NSbands = 4; // number of subbands in 2D
//  if (Lvl >= Size(*Vols))
//    return; // base case, end the recursive calls
//  /* transform the current tile */
//  v3i& M = (*RDims3s)[Lvl]; // dims of the current tile
//  mg_Assert(M.Z == 1);
//  if (!Last)
//    mg_Assert(IsEven(M.X) && IsEven(M.Y));
//  FLiftCdf53X<f64>(grid(M), M, lift_option::NoUpdateLast, Vol);
//  M.X += IsEven(M.X);
//  FLiftCdf53Y<f64>(grid(M), M, lift_option::NoUpdateLast, Vol);
//  M.Y += IsEven(M.Y);
//  /* initialize the memory for the parent tiles if this is the first children */
//  stack_linear_allocator<4> Alloc;
//  array<extent> Sbands(&Alloc);
//  BuildSubbands(M, 1, &Sbands);
//  int NLvl = Lvl + 1;
//  if (NIdx == 0) {
//    Resize(&(*Vols)[NLvl], NSbands);
//    Resize(&(*RDims3s)[NLvl], NSbands);
//  }
//  u32 NIdx = Idx % NSbands; // normalized index
//  v3i HDims3 = (TDims3 + 1) / 2; // half dims
//  v3i P((NIdx & 1) * HDims3.X, (NIdx & 2) * HDims3.Y, 1);
//  /* spread the samples to the 4 parent subbands */
//  for (int Sb = 0; Sb < NSbands; ++Sb) {
//    if (NIdx == 0) { // first children, allocate memory
//      buffer Buf;
//      AllocBuf0(&Buf, sizeof(f64) * Prod(TDims3.XY + 1));
//      (*Vols)[NLvl][Sb] = volume(TDims3.XY + 1, Buf);
//      (*RDims3s)[NLvl][Sb] = v3i::Zero;
//    }
//    /* copy the samples */
//    Add(Sbands[Sb], (*Vols)[Lvl][NIdx], extent(P, Dims(Sbands[Sb])), (*Vols)[NLvl][Sb]);
//    /* update the dimensions */
//    v3i& PDims3 = (*RDims3s)[NLvl][Sb];
//    if (P.X == 0 && P.Y == 0)
//      PDims3 += Dims(Sbands[Sb]);
//    else if (P.X > 0 && P.Y == 0)
//      PDims3.X += Dims(Sbands[Sb]).X;
//    else if (P.X == 0 && P.Y > 0)
//      PDims3.Y += Dims(Sbands[Sb]).Y;
//  }
//  ZeroBuf(&((*Vols)[Lvl][NIdx].Buffer)); // clear the current tile's buffer
//  /* if this is the last children, recurse */
//  if (Last || NIdx + 1 == NSbands)
//    ForwardCdf53Tile2D(NLvl, Idx / NSbands, Last, TDims3, RDims3s, Vols);
//}

//void
//ForwardCdf53Tile(
//  int NLvls, // number of levels
//  const v3i& TDims3, // dimensions of a tile (e.g. 32 x 32)
//  volume* Vol) // big volume of data
//{
//  /* calculate the power-of-two dimensions encompassing the volume */
//  v3i Dims3 = Dims(*Vol);
//  v3i BigDims3 = v3i::One;
//  while (BigDims3.X < Dims3.X || BigDims3.Y < Dims3.Y || BigDims3.Z < Dims3.Z)
//    BigDims3 = BigDims3 * 2;
//  /* loop through the tiles in Z (morton) order */
//  v3i NTiles3 = (BigDims3 + TDims3 - 1) / TDims3;
//  for (int Idx = 0; I) {
//
//  }
//}

void
ForwardCdf53(const extent& Ext, int NLevels, volume* Vol) {
#define Body(type)\
  v3i Dims3 = Dims(Ext), M = Dims(Ext), Strd3 = v3i::One;\
  array<grid> Grids;\
  for (int I = 0; I < NLevels; ++I) {\
    PushBack(&Grids, grid(v3i::Zero, Dims3, Strd3));\
    Dims3.X += IsEven(Dims3.X); /* extrapolate */\
    PushBack(&Grids, grid(v3i::Zero, Dims3, Strd3));\
    Dims3.Y += IsEven(Dims3.Y); /* extrapolate */\
    PushBack(&Grids, grid(v3i::Zero, Dims3, Strd3));\
    Dims3.Z += IsEven(Dims3.Z); /* extrapolate */\
    Strd3 = Strd3 * 2;\
    Dims3 = (Dims3 + 1) / 2;\
  }\
  for (int I = 0, J = 0; I < NLevels; ++I) {\
    FLiftCdf53X<type>(Grids[J++], M, lift_option::Normal, Vol);\
    FLiftCdf53Y<type>(Grids[J++], M, lift_option::Normal, Vol);\
    FLiftCdf53Z<type>(Grids[J++], M, lift_option::Normal, Vol);\
  }

  mg_DispatchOnType(Vol->Type)
#undef Body
}

void
InverseCdf53(const extent& Ext, int NLevels, volume* Vol) {
#define Body(type)\
  v3i Dims3 = Dims(Ext), M = Dims(Ext), Strd3 = v3i::One;\
  array<grid> Grids;\
  for (int I = 0; I < NLevels; ++I) {\
    PushBack(&Grids, grid(v3i::Zero, Dims3, Strd3));\
    Dims3.X += IsEven(Dims3.X); /* extrapolate */\
    PushBack(&Grids, grid(v3i::Zero, Dims3, Strd3));\
    Dims3.Y += IsEven(Dims3.Y); /* extrapolate */\
    PushBack(&Grids, grid(v3i::Zero, Dims3, Strd3));\
    Dims3.Z += IsEven(Dims3.Z); /* extrapolate */\
    Strd3 = Strd3 * 2;\
    Dims3 = (Dims3 + 1) / 2;\
  }\
  for (int I = NLevels - 1, J = Size(Grids) - 1; I >= 0; --I) {\
    ILiftCdf53Z<type>(Grids[J--], M, lift_option::Normal, Vol);\
    ILiftCdf53Y<type>(Grids[J--], M, lift_option::Normal, Vol);\
    ILiftCdf53X<type>(Grids[J--], M, lift_option::Normal, Vol);\
  }

  mg_DispatchOnType(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void
InverseCdf53Old(volume* Vol, int NLevels) {
#define Body(type)\
  v3i Dims3 = Dims(*Vol);\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  for (int I = NLevels - 1; I >= 0; --I) {\
    ILiftCdf53OldZ(FPtr, Dims3, v3i(I));\
    ILiftCdf53OldY(FPtr, Dims3, v3i(I));\
    ILiftCdf53OldX(FPtr, Dims3, v3i(I));\
  }\

  mg_DispatchOnType(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void
ForwardCdf53Ext(const extent& Ext, volume* Vol) {
#define Body(type)\
  v3i N = Dims(Ext);\
  v3i NN = Dims(*Vol);\
  if (NN.Y > 1) mg_Assert(NN.X == NN.Y);\
  if (NN.Z > 1) mg_Assert(NN.Y == NN.Z);\
  mg_Assert(IsPow2(NN.X - 1));\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  int NLevels = Log2Floor(NN.X - 1) + 1;\
  for (int I = 0; I < NLevels; ++I) {\
    FLiftExtCdf53X(FPtr, N, NN, v3i(I));\
    FLiftExtCdf53Y(FPtr, N, NN, v3i(I));\
    FLiftExtCdf53Z(FPtr, N, NN, v3i(I));\
  }\

  mg_DispatchOnType(Vol->Type)
#undef Body
}

// TODO: this won't work for a general (sub)volume
void
InverseCdf53Ext(const extent& Ext, volume* Vol) {
#define Body(type)\
  v3i N = Dims(Ext);\
  v3i NN = Dims(*Vol);\
  if (NN.Y > 1) mg_Assert(NN.X == NN.Y);\
  if (NN.Z > 1) mg_Assert(NN.Y == NN.Z);\
  mg_Assert(IsPow2(NN.X - 1));\
  type* FPtr = (type*)(Vol->Buffer.Data);\
  int NLevels = Log2Floor(NN.X - 1) + 1;\
  for (int I = NLevels - 1; I >= 0; --I) {\
    ILiftExtCdf53Z(FPtr, N, NN, v3i(I));\
    ILiftExtCdf53Y(FPtr, N, NN, v3i(I));\
    ILiftExtCdf53X(FPtr, N, NN, v3i(I));\
  }\

  mg_DispatchOnType(Vol->Type)
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
BuildSubbands(const v3i& N, int NLevels, array<extent>* Subbands) {
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
        PushBack(Subbands, extent(v3i(X, Y, Z) * P, Sm));
      else if (NDims == 2 && Sm.X != 0 && Sm.Y != 0)
        PushBack(Subbands, extent(v3i(X * P.X, Y * P.Y, 0), v3i(Sm.X, Sm.Y, 1)));
      else
        PushBack(Subbands, extent(v3i(X * P.X, 0, 0), v3i(Sm.X, 1, 1)));
    }
    M = P;
  }
  PushBack(Subbands, extent(v3i(0), M));
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
FormSubbands(int NLevels, const volume& SVol, volume* DVol) {
  mg_Assert(SVol.Dims == DVol->Dims);
  mg_Assert(SVol.Type == DVol->Type);
  v3i Dims3 = Dims(SVol);
  array<extent> Subbands;
  array<grid> SubbandsInPlace;
  BuildSubbands(Dims3, NLevels, &Subbands);
  BuildSubbandsInPlace(Dims3, NLevels, &SubbandsInPlace);
  for (int I = 0; I < Size(Subbands); ++I)
    Copy(SubbandsInPlace[I], SVol, Subbands[I], DVol);
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


