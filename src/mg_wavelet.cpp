#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_wavelet.h"
#include "mg_logger.h"
#include "robin_hood.h"

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
  }

  mg_DispatchOnType(Vol->Type)
#undef Body
}

struct tile_buf {
  i8 NDeps = 0; // number of dependent tiles
  i8 MDeps = 0; // maximum number of dependencies
  volume Vol = {}; // storing tile data
};
#include <unordered_map>
// they key is the row major index of the tile
using tile_map = robin_hood::unordered_map<i64, tile_buf>;
//using tile_map = std::unordered_map<i64, tile_buf>;

// TODO: replace f64 with a template parameter
// TODO: check if things work when we have only one sample in the block in either X, Y, Z
// TODO: test the Add/Copy functions in many situations
void
ForwardCdf53Tile(
  const v3i& TDims3, // dimensions of a tile (e.g. 32 x 32 x 32)
  int Lvl, // level of the current tile
  const v3i& Pos3, // index of the tile in each dimension
  const array<v3i>& Dims3s, // dimensions of the big array on each level
  array<array<tile_map>>* Vols) // level -> subband -> tiles
{
  mg_Assert(IsEven(TDims3.X) && IsEven(TDims3.Y) && IsEven(TDims3.Z));
  int NLevels = Size(*Vols) - 1;
  mg_Assert(Size(Dims3s) == NLevels + 1);
  mg_Assert(Lvl <= NLevels);
  const int NSbands = 8; // number of subbands in 3D
  /* transform the current tile */
  v3i NTiles3 = (Dims3s[Lvl] + TDims3 - 1) / TDims3;
  mg_Assert(Pos3 < NTiles3);
  // TODO: check the use of M below
  v3i M(Min(TDims3, v3i(Dims3s[Lvl] - Pos3 * TDims3))); // dims of the current tile
  volume Vol = (*Vols)[Lvl][0][Row(NTiles3, Pos3)].Vol; // TODO: check for existence first?
  if (Pos3.X + 1 < NTiles3.X)
    FLiftCdf53X<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
  else // last tile in X
    FLiftCdf53X<f64>(grid(M), M, lift_option::Normal, &Vol);
  M.X += IsEven(M.X);
  if (Pos3.Y + 1 < NTiles3.Y)
    FLiftCdf53Y<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
  else // last tile in Y
    FLiftCdf53Y<f64>(grid(M), M, lift_option::Normal, &Vol);
  M.Y += IsEven(M.Y);
  if (Pos3.Z + 1 < NTiles3.Z)
    FLiftCdf53Z<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
  else // last tile in Z
    FLiftCdf53Z<f64>(grid(M), M, lift_option::Normal, &Vol);
  M.Z += IsEven(M.Z);
  /* end the recursion if this is the last level */
  int LvlNext = Lvl + 1;
  if (LvlNext > NLevels)
    return; // TODO: just return?
  stack_linear_allocator<8 * sizeof(extent)> Alloc;
  array<extent> MySbands(&Alloc);
  BuildSubbands(TDims3 + 1, 1, &MySbands);
  stack_linear_allocator<8 * sizeof(grid)> Alloc2;
  array<grid> MySbandsInPlace(&Alloc2);
  BuildSubbandsInPlace(TDims3 + 1, 1, &MySbandsInPlace);
  /* spread the samples to the parent subbands */
  v3i Dims3Next = Dims3s[LvlNext];
  v3i NTiles3Next = (Dims3Next + TDims3 - 1) / TDims3;
  for (int Sb = 0; Sb < NSbands; ++Sb) {
    v3i D = Pos3 - (Pos3 / 2) * 2; // either 0 or 1 in each dimension
    D = D * 2 - 1; // map [0, 1] to [-1, 1]
    extent DstG = MySbands[Sb];
    grid SrcG = MySbandsInPlace[Sb];
    for (int Z = 0, Iz = 0; Iz < 2; Z += D.Z, ++Iz) { // (next-level) neighbor loop
      grid SrcGZ = SrcG;
      extent DstGZ = DstG;
      if (Z != 0)  {
        SrcGZ = Slab(SrcGZ, dimension::Z, -Z);
        DstGZ = Slab(DstGZ, dimension::Z,  Z);
        if (Z == 1) Translate(DstGZ, dimension::Z, TDims3.Z / 2);
      }
      for (int Y = 0, Iy = 0; Iy < 2; Y += D.Y, ++Iy) {
        grid SrcGY = SrcGZ;
        extent DstGY = DstGZ;
        if (Y != 0) {
          SrcGY = Slab(SrcGY, dimension::Y, -Y);
          DstGY = Slab(DstGY, dimension::Y,  Y);
          if (Y == 1) Translate(DstGY, dimension::Y, TDims3.Y / 2);
        }
        for (int X = 0, Ix = 0; Ix < 2; X += D.X, ++Ix) {
          grid SrcGX = SrcGY;
          extent DstGX = DstGY;
          if (X != 0) {
            SrcGX = Slab(SrcGX, dimension::X, -X);
            DstGX = Slab(DstGX, dimension::X,  X);
            if (X == 1) Translate(DstGX, dimension::X, TDims3.X / 2);
          }
          v3i Pos3Next = Pos3 / 2 + v3i(X, Y, Z);
          if (!(Pos3Next >= v3i::Zero && Pos3Next < NTiles3Next))
            continue; // tile outside the domain
          tile_buf& TileNext = (*Vols)[LvlNext][Sb][Row(NTiles3Next, Pos3Next)];
          volume& DVol = TileNext.Vol;
          //mg_Log("out.txt", "Next Vol %d %d %d ", LvlNext, Sb, (int)Row(NTiles3Next, Pos3Next));
          if (!DVol.Buffer) {
            buffer Buf;
            AllocBuf0(&Buf, sizeof(f64) * Prod(TDims3 + 1));
            DVol = volume(Buf, TDims3 + 1, dtype::float64);
            Add(SrcG, Vol, DstG, &DVol);
          }
          if (TileNext.MDeps == 0) { // compute the number of dependencies
            v3i MDeps3(4, 4, 4); // by default each tile depends on 64 finer tiles
            for (int I = 0; I < 3; ++I) {
              MDeps3[I] -= Pos3Next[I] == 0;
              MDeps3[I] -= Pos3Next[I] == NTiles3Next[I] - 1;
              MDeps3[I] -= Dims3Next[I] - Pos3Next[I] * TDims3[I] <= TDims3[I] / 2;
            }
            TileNext.MDeps = Prod(MDeps3);
          }
          ++TileNext.NDeps;
          if (Sb == 0 && TileNext.MDeps == TileNext.NDeps) { // recurse
            ForwardCdf53Tile(TDims3, LvlNext, Pos3Next, Dims3s, Vols);
          }
        }
      }
    } // end neighbor loop
  } // end subband loop
  // TODO: dealloc vols that are not in the LLL subband
  DeallocBuf(&Vol.Buffer);
  (*Vols)[Lvl][0].erase(Row(NTiles3, Pos3)); // TODO: remove via iterator?
}

// TODO: replace f64 with a generic type
void
ForwardCdf53Tile(int NLvls, const v3i& TDims3, volume* Vol) {
  /* calculate the power-of-two dimensions encompassing the volume */
  v3i M = Dims(*Vol);
  v3i N = v3i::One;
  while (N.X < M.X || N.Y < M.Y || N.Z < M.Z)
    N = N * 2;
  /* loop through the tiles in Z (morton) order */
  array<v3i> Dims3s;
  Init(&Dims3s, NLvls + 1);
  for (int I = 0; I < Size(Dims3s); ++I) {
    M = M + IsEven(M);
    Dims3s[I] = M;
    M = (M + 1) / 2;
  }
  array<array<tile_map>> Vols;
  Init(&Vols, NLvls + 1);
  for (int I = 0; I < Size(Vols); ++I) {
    Vols[I] = array<tile_map>();
    Init(&Vols[I], 8);
    for (int J = 0; J < Size(Vols[I]); ++J) {
      new (&Vols[I][J]) tile_map;
      //Vols[I][J] = tile_map(); // TODO: replace with placement new
    }
  }
  //Vols[0][0] = *Vol;
  M = Dims(*Vol);
  v3i NTiles3 = (M + IsEven(M) + TDims3 - 1) / TDims3;
  v3i NTilesBig3 = (N + TDims3 - 1) / TDims3;
  for (u32 I = 0; I < Prod<u32>(NTilesBig3); ++I) {
    u32 X = DecodeMorton3X(I), Y = DecodeMorton3Y(I), Z = DecodeMorton3Z(I);
    v3i Pos3(X, Y, Z);
    i64 Idx = Row(NTiles3, Pos3);
    buffer Buf;
    AllocBuf0(&Buf, Prod(TDims3 + 1) * sizeof(f64));
    volume& TileVol = Vols[0][0][Idx].Vol;
    TileVol = volume(Buf, TDims3 + 1, dtype::float64);
    extent E(Pos3 * TDims3, TDims3 + 1);
    v3i From3 = From(E);
    v3i Dims3 = Min(Dims(E), M - From3);
    SetDims(E, Dims3);
    if (!(From3 < M)) // tile outside domain
      continue;
    Copy(E, *Vol, extent(v3i::Zero, Dims(E)), &TileVol);
    ForwardCdf53Tile(TDims3, 0, Pos3, Dims3s, &Vols);
  }
}

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
  }

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
  }

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
  }

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
  return 7 * (Lvl - 1) +
         Table[4 * (Level.X == Lvl) + 2 * (Level.Y == Lvl) + 1 * (Level.Z == Lvl)];
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


