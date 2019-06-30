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
// they key is the row major index of the tile
using tile_map = robin_hood::unordered_map<i64, tile_buf>;
//using tile_map = std::map<i64, tile_buf>;

#define mg_Cdf53TileDebug

void
ForwardCdf53Tile2D(
  const v3i& TDims3, // dimensions of a tile (e.g. 32 x 32 x 32)
  int Lvl, // level of the current tile
  const v3i& Pos3, // index of the tile in each dimension (within a subband)
  const array<v3i>& Dims3s, // dimensions of the big array on each level
  array<array<tile_map>>* Vols // level -> subband -> tiles
#if defined(mg_Cdf53TileDebug)
  , volume* BigVol // we will copy the coefficients here to debug
  , const array<extent>& BigSbands // subbands of the big volume
#endif
)
{
  mg_Assert(IsEven(TDims3.X) && IsEven(TDims3.Y));
  int NLevels = Size(*Vols) - 1;
  mg_Assert(Size(Dims3s) == NLevels + 1);
  mg_Assert(Lvl <= NLevels);
  const int NSbands = 4; // number of subbands in 2D
  /* transform the current tile */
  v3i NTiles3 = (Dims3s[Lvl] + TDims3 - 1) / TDims3;
  mg_Assert(Pos3 < NTiles3);
  v3i M(Min(TDims3, v3i(Dims3s[Lvl] - Pos3 * TDims3))); // dims of the current tile
  volume Vol = (*Vols)[Lvl][0][Row(NTiles3, Pos3)].Vol;
  mg_Assert(Vol.Buffer);
  int LvlNxt = Lvl + 1;
  if (LvlNxt <= NLevels) {
    M.X += IsEven(M.X);
    M.Y += IsEven(M.Y);
    if (M.X > 1) {
      if (Pos3.X + 1 < NTiles3.X) // not last tile in X
        FLiftCdf53X<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
      else // last tile in X
        FLiftCdf53X<f64>(grid(M), M, lift_option::Normal, &Vol);
    }
    if (M.Y > 1) {
      if (Pos3.Y + 1 < NTiles3.Y) // not last tile
        FLiftCdf53Y<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
      else // last tile in Y
        FLiftCdf53Y<f64>(grid(M), M, lift_option::Normal, &Vol);
    }
  }
  /* end the recursion if this is the last level */
  if (LvlNxt > NLevels) { // last level, no need to add to the next level
#if defined(mg_Cdf53TileDebug)
    v3i CopyM = Min(M, TDims3);
    Copy(extent(CopyM), Vol, extent(Pos3 * TDims3, CopyM), BigVol);
#endif
    DeallocBuf(&Vol.Buffer);
    (*Vols)[Lvl][0].erase(Row(NTiles3, Pos3));
    return;
  }
  v3i TDims3Ext = TDims3 + v3i(1, 1, 0);
  stack_linear_allocator<4 * sizeof(grid)> Alloc;
  array<grid> Sbands(&Alloc); BuildSubbandsInPlace(TDims3Ext, 1, &Sbands);
  /* spread the samples to the parent subbands */
  v3i Dims3Next = Dims3s[LvlNxt];
  v3i NTiles3Nxt = (Dims3Next + TDims3 - 1) / TDims3;
  for (int Sb = 0; Sb < NSbands; ++Sb) { // through subbands
    v3i D01 = Pos3 - (Pos3 / 2) * 2; // either 0 or 1 in each dimension
    v3i D11 = D01 * 2 - 1; // map [0, 1] to [-1, 1]
    grid SrcG = Sbands[Sb];
    extent DstG(v3i::Zero, Dims(SrcG));
    v2i L = SubbandToLevel2(Sb);
    for (int Y = 0, Iy = 0; Iy < 2; Y += D11.Y, ++Iy) {
      if (Y == 1 && L.Y == 1)
        continue;
      grid SrcGY = SrcG;
      extent DstGY = DstG;
      if (Y != 0) { // contributing to top/bottom parents, take 1 slab only
        if (Y == -1) // contributing to the top parent tile, so shift down full
          DstGY = Translate(DstGY, dimension::Y, TDims3.Y);
        SrcGY = Slab(SrcGY, dimension::Y, -Y);
        DstGY = Slab(DstGY, dimension::Y,  1);
      } else if (D01.Y == 1) { // second child, shift down half
        DstGY = Translate(DstGY, dimension::Y, TDims3.Y / 2);
      }
      for (int X = 0, Ix = 0; Ix < 2; X += D11.X, ++Ix) {
        if (X == 1 && L.X == 1)
          continue;
        grid SrcGX = SrcGY;
        extent DstGX = DstGY;
        if (X != 0) { // contributing to left/right parents, take 1 slab only
          if (X == -1) // contributing to left parent, shift right full
            DstGX = Translate(DstGX, dimension::X, TDims3.X);
          SrcGX = Slab(SrcGX, dimension::X, -X);
          DstGX = Slab(DstGX, dimension::X,  1);
        } else if (D01.X == 1) { // second child, shift right half
          DstGX = Translate(DstGX, dimension::X, TDims3.X / 2);
        }
        /* locate the finer tile */
        v3i Pos3Nxt = Pos3 / 2 + v3i(X, Y, 0);
        if (!(Pos3Nxt >= v3i::Zero && Pos3Nxt < NTiles3Nxt))
          continue; // tile outside the domain
        tile_buf& TileNxt = (*Vols)[LvlNxt][Sb][Row(NTiles3Nxt, Pos3Nxt)];
        volume& VolNxt = TileNxt.Vol;
        /* add contribution to the finer tile, allocating its memory if needed */
        if (TileNxt.MDeps == 0) {
          mg_Assert(TileNxt.NDeps == 0);
          mg_Assert(!VolNxt.Buffer);
          buffer Buf; AllocBuf0(&Buf, sizeof(f64) * Prod(TDims3Ext));
          VolNxt = volume(Buf, TDims3Ext, dtype::float64);
          /* compute the number of dependencies for the finer tile if necessary */
          v3i MDeps3(4, 4, 1); // by default each tile depends on 16 finer tiles
          for (int I = 0; I < 2; ++I) {
            MDeps3[I] -= Pos3Nxt[I] == 0;
            MDeps3[I] -= Pos3Nxt[I] == NTiles3Nxt[I] - 1;
            MDeps3[I] -= Dims3Next[I] - Pos3Nxt[I] * TDims3[I] <= TDims3[I] / 2;
          }
          TileNxt.MDeps = Prod(MDeps3);
        }
        Add(SrcGX, Vol, DstGX, &VolNxt);
        ++TileNxt.NDeps;
        /* if the finer tile receives from all its dependencies, recurse */
        if (TileNxt.MDeps == TileNxt.NDeps) {
          if (Sb == 0) { // recurse
#if defined(mg_Cdf53TileDebug)
            ForwardCdf53Tile2D(TDims3, LvlNxt, Pos3Nxt, Dims3s, Vols, BigVol, BigSbands);
#else
            ForwardCdf53Tile(TDims3, LvlNxt, Pos3Nxt, Dims3s, Vols);
#endif
          } else { // copy data to the big buffer, for testing
#if defined(mg_Cdf53TileDebug)
            int BigSb = (NLevels - LvlNxt) * 3 + Sb;
            v3i F3 = From(BigSbands[BigSb]);
            v3i D3 = Dims(BigSbands[BigSb]);
            F3 = F3 + Pos3Nxt * TDims3;
            // NOTE: for subbands other than 0, F3 can be outside of the big volume
            mg_Assert(VolNxt.Buffer);
            if (F3 < Dims(*BigVol)) {
              v3i CopyDims3 = Min(Dims(*BigVol) - F3, TDims3);
              mg_Assert(F3 + CopyDims3 <= Dims(*BigVol));
              Copy(extent(CopyDims3), VolNxt, extent(F3, CopyDims3), BigVol);
            }
#endif
            DeallocBuf(&VolNxt.Buffer);
            (*Vols)[LvlNxt][Sb].erase(Row(NTiles3Nxt, Pos3Nxt));
          }
        }
      } // end X loop
    } // end Y loop
  } // end subband loop
  DeallocBuf(&Vol.Buffer);
  (*Vols)[Lvl][0].erase(Row(NTiles3, Pos3));
}

void
ForwardCdf53Tile2D(int NLvls, const v3i& TDims3, const volume& Vol
#if defined(mg_Cdf53TileDebug)
  , volume* OutVol
#endif
) {
  /* calculate the power-of-two dimensions encompassing the volume */
  v3i M = Dims(Vol);
  v3i N(v2i::One, 1);
  while (N.X < M.X || N.Y < M.Y)
    N = N * 2;
  N.Z = 1;
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
    Init(&Vols[I], 4);
    for (int J = 0; J < Size(Vols[I]); ++J)
      new (&Vols[I][J]) tile_map;
  }
  M = Dims(Vol);
  v3i NTiles3 = (M + IsEven(M) + TDims3 - 1) / TDims3;
  v3i NTilesBig3 = (N + TDims3 - 1) / TDims3;
  v3i TDims3Ext = TDims3 + v3i(1, 1, 0);
  array<extent> BigSbands; BuildSubbands(M, NLvls, &BigSbands);
  for (u32 I = 0; I < Prod<u32>(NTilesBig3); ++I) {
    // TODO: count the number of tiles processed and break if we are done
    u32 X = DecodeMorton2X(I), Y = DecodeMorton2Y(I);
    v3i Pos3(X, Y, 0);
    if (!(Pos3 * TDims3 < M)) // tile outside the domain
      continue;
    i64 Idx = Row(NTiles3, Pos3);
    buffer Buf; AllocBuf0(&Buf, Prod(TDims3Ext) * sizeof(f64));
    volume& TileVol = Vols[0][0][Idx].Vol;
    TileVol = volume(Buf, TDims3Ext, dtype::float64);
    extent E(Pos3 * TDims3, TDims3Ext);
    v3i From3 = From(E);
    v3i Dims3 = Min(Dims(E), M - From3);
    SetDims(E, Dims3);
    if (!(From3 < M)) // tile outside domain
      continue;
    Copy(E, Vol, extent(v3i::Zero, Dims(E)), &TileVol);
    ForwardCdf53Tile2D(TDims3, 0, Pos3, Dims3s, &Vols
#if defined(mg_Cdf53TileDebug)
      , OutVol, BigSbands
#endif
    );
  }
}

// TODO: replace f64 with a template parameter
// TODO: check if things work when we have only one sample in the block in either X, Y, Z
// TODO: check the computation of Dims3s
void
ForwardCdf53Tile(
  const v3i& TDims3, // dimensions of a tile (e.g. 32 x 32 x 32)
  int Lvl, // level of the current tile
  const v3i& Pos3, // index of the tile in each dimension (within a subband)
  const array<v3i>& Dims3s, // dimensions of the big array on each level
  array<array<tile_map>>* Vols // level -> subband -> tiles
#if defined(mg_Cdf53TileDebug)
  , volume* BigVol // we will copy the coefficients here to debug
  , const array<extent>& BigSbands // subbands of the big volume
#endif
)
{
  static int Counter = 0;
  ++Counter;
  mg_Assert(IsEven(TDims3.X) && IsEven(TDims3.Y) && IsEven(TDims3.Z));
  int NLevels = Size(*Vols) - 1;
  mg_Assert(Size(Dims3s) == NLevels + 1);
  mg_Assert(Lvl <= NLevels);
  const int NSbands = 8; // number of subbands in 3D
  /* transform the current tile */
  v3i NTiles3 = (Dims3s[Lvl] + TDims3 - 1) / TDims3;
  mg_Assert(Pos3 < NTiles3);
  v3i M(Min(TDims3, v3i(Dims3s[Lvl] - Pos3 * TDims3))); // dims of the current tile
  volume Vol = (*Vols)[Lvl][0][Row(NTiles3, Pos3)].Vol;
  mg_Assert(Vol.Buffer);
  //if (Pos3.X + 1 < NTiles3.X)
  //  FLiftCdf53X<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
  //else if (M.X > 1)// last tile in X
  //  FLiftCdf53X<f64>(grid(M), M, lift_option::Normal, &Vol);
  M.X += IsEven(M.X);
  //if (Pos3.Y + 1 < NTiles3.Y)
  //  FLiftCdf53Y<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
  //else if (M.Y > 1)// last tile in Y
  //  FLiftCdf53Y<f64>(grid(M), M, lift_option::Normal, &Vol);
  M.Y += IsEven(M.Y);
  //if (Pos3.Z + 1 < NTiles3.Z)
  //  FLiftCdf53Z<f64>(grid(M), M, lift_option::PartialUpdateLast, &Vol);
  //else if (M.Z > 1)// last tile in Z
  //  FLiftCdf53Z<f64>(grid(M), M, lift_option::Normal, &Vol);
  M.Z += IsEven(M.Z);
  /* end the recursion if this is the last level */
  int LvlNxt = Lvl + 1;
  if (LvlNxt > NLevels) { // last level, no need to add to the next level
#if defined(mg_Cdf53TileDebug)
    v3i MM = Pos3 * TDims3;
    printf("copy1 (%d %d %d) (%d %d %d)\n", M.X, M.Y, M.Z, MM.X, MM.Y, MM.Z);
    Copy(extent(M), Vol, extent(Pos3 * TDims3, M), BigVol);
#endif
    DeallocBuf(&Vol.Buffer);
    (*Vols)[Lvl][0].erase(Row(NTiles3, Pos3));
    //printf("deleting %d %d (%d %d %d) (%d %d %d) %d\n", Lvl, 0,
    //  NTiles3.X, NTiles3.Y, NTiles3.Z, Pos3.X, Pos3.Y, Pos3.Z, Row(NTiles3, Pos3));
    return;
  }
  stack_linear_allocator<16 * sizeof(grid)> Alloc;
  array<extent> Sbands(&Alloc); BuildSubbands(TDims3 + 1, 1, &Sbands);
  array<grid> SbandsInPlc(&Alloc); BuildSubbandsInPlace(TDims3 + 1, 1, &SbandsInPlc);
  /* spread the samples to the parent subbands */
  v3i Dims3Next = Dims3s[LvlNxt];
  v3i NTiles3Nxt = (Dims3Next + TDims3 - 1) / TDims3;
  for (int Sb = 0; Sb < NSbands; ++Sb) { // through subbands
    v3i D = Pos3 - (Pos3 / 2) * 2; // either 0 or 1 in each dimension
    D = D * 2 - 1; // map [0, 1] to [-1, 1]
    extent DstG = Sbands[Sb];
    grid SrcG = SbandsInPlc[Sb];
    for (int Z = 0, Iz = 0; Iz < 2; Z += D.Z, ++Iz) { // through (next-level) neighbors
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
          /* locate the finer tile */
          v3i Pos3Nxt = Pos3 / 2 + v3i(X, Y, Z);
          if (!(Pos3Nxt >= v3i::Zero && Pos3Nxt < NTiles3Nxt))
            continue; // tile outside the domain
          tile_buf& TileNxt = (*Vols)[LvlNxt][Sb][Row(NTiles3Nxt, Pos3Nxt)];
          //printf("accessing %d %d (%d %d %d) (%d %d %d) %d\n", LvlNxt, Sb,
          //       NTiles3Nxt.X, NTiles3Nxt.Y, NTiles3Nxt.Z,
          //       Pos3Nxt.X, Pos3Nxt.Y, Pos3Nxt.Z, Row(NTiles3Nxt, Pos3Nxt));
          volume& VolNxt = TileNxt.Vol;
          /* add contribution to the finer tile, allocating its memory if needed */
          if (TileNxt.MDeps == 0) {
            mg_Assert(TileNxt.NDeps == 0);
            mg_Assert(!VolNxt.Buffer);
            buffer Buf; AllocBuf0(&Buf, sizeof(f64) * Prod(TDims3 + 1));
            VolNxt = volume(Buf, TDims3 + 1, dtype::float64);
            //printf("allocating %d %d (%d %d %d) (%d %d %d) %d\n", LvlNxt, Sb,
            //       NTiles3Nxt.X, NTiles3Nxt.Y, NTiles3Nxt.Z,
            //       Pos3Nxt.X, Pos3Nxt.Y, Pos3Nxt.Z, Row(NTiles3Nxt, Pos3Nxt));
            /* compute the number of dependencies for the finer tile if necessary */
            v3i MDeps3(4, 4, 4); // by default each tile depends on 64 finer tiles
            for (int I = 0; I < 3; ++I) {
              MDeps3[I] -= Pos3Nxt[I] == 0;
              MDeps3[I] -= Pos3Nxt[I] == NTiles3Nxt[I] - 1;
              // TODO: check this
              MDeps3[I] -= Dims3Next[I] - Pos3Nxt[I] * TDims3[I] <= TDims3[I] / 2;
            }
            TileNxt.MDeps = Prod(MDeps3);
          }
          Add(SrcG, Vol, DstG, &VolNxt);
          ++TileNxt.NDeps;
          /* if the finer tile receives from all its dependencies, recurse */
          if (TileNxt.MDeps == TileNxt.NDeps) {
            if (Sb == 0) { // recurse
#if defined(mg_Cdf53TileDebug)
              ForwardCdf53Tile(TDims3, LvlNxt, Pos3Nxt, Dims3s, Vols, BigVol, BigSbands);
#else
              ForwardCdf53Tile(TDims3, LvlNxt, Pos3Nxt, Dims3s, Vols);
#endif
            } else { // copy data to the big buffer, for testing
#if defined(mg_Cdf53TileDebug)
              int BigSb = (NLevels - LvlNxt) * 7 + Sb;
              v3i F3 = From(BigSbands[BigSb]);
              F3 = F3 + Pos3Nxt * TDims3;
              // NOTE: for subbands other than 0, F3 can be outside of the big volume
              mg_Assert(VolNxt.Buffer);
              if (F3 < Dims(*BigVol)) {
                mg_Assert(F3 + M <= Dims(*BigVol));
                printf("copy2 (%d %d %d) (%d %d %d)\n", M.X, M.Y, M.Z, F3.X, F3.Y, F3.Z);
                Copy(extent(M), VolNxt, extent(F3, M), BigVol);
              }
#endif
              DeallocBuf(&VolNxt.Buffer);
              (*Vols)[LvlNxt][Sb].erase(Row(NTiles3Nxt, Pos3Nxt));
            }
          }
        } // end X loop
      } // end Y loop
    } // end neighbor loop
  } // end subband loop
  DeallocBuf(&Vol.Buffer);
  (*Vols)[Lvl][0].erase(Row(NTiles3, Pos3));
}

// TODO: replace f64 with a generic type
void
ForwardCdf53Tile(int NLvls, const v3i& TDims3, const volume& Vol
#if defined(mg_Cdf53TileDebug)
  , volume* OutVol
#endif
) {
  /* calculate the power-of-two dimensions encompassing the volume */
  v3i M = Dims(Vol);
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
    for (int J = 0; J < Size(Vols[I]); ++J)
      new (&Vols[I][J]) tile_map;
  }
  M = Dims(Vol);
  v3i NTiles3 = (M + IsEven(M) + TDims3 - 1) / TDims3;
  v3i NTilesBig3 = (N + TDims3 - 1) / TDims3;
  array<extent> BigSbands; BuildSubbands(M, NLvls, &BigSbands);
  for (u32 I = 0; I < Prod<u32>(NTilesBig3); ++I) {
    u32 X = DecodeMorton3X(I), Y = DecodeMorton3Y(I), Z = DecodeMorton3Z(I);
    v3i Pos3(X, Y, Z);
    if (!(Pos3 < M)) // tile outside the domain
      continue;
    i64 Idx = Row(NTiles3, Pos3);
    buffer Buf; AllocBuf0(&Buf, Prod(TDims3 + 1) * sizeof(f64));
    volume& TileVol = Vols[0][0][Idx].Vol;
    TileVol = volume(Buf, TDims3 + 1, dtype::float64);
    extent E(Pos3 * TDims3, TDims3 + 1);
    v3i From3 = From(E);
    v3i Dims3 = Min(Dims(E), M - From3);
    SetDims(E, Dims3);
    if (!(From3 < M)) // tile outside domain
      continue;
    Copy(E, Vol, extent(v3i::Zero, Dims(E)), &TileVol);
    ForwardCdf53Tile(TDims3, 0, Pos3, Dims3s, &Vols
#if defined(mg_Cdf53TileDebug)
      , OutVol, BigSbands
#endif
    );
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

v2i
SubbandToLevel2(int S) {
  if (S == 0)
    return v2i(0, 0);
  /* handle level 0 which has only 1 subband (other levels have 7 subbands) */
  int Lvl = (S + 2) / 3;
  /* subtract all subbands on previous levels (except the subband 0);
  basically it reduces the case to the 2x2 case where subband 0 is in corner */
  S -= 3 * (Lvl - 1);
  /* bit 0 -> y axis offset; bit 1 -> x axis offset
  we subtract from Lvl as it corresponds to the +x, +y corner */
  return v2i(Lvl - !BitSet(S, 1), Lvl - !BitSet(S, 0));
}

v3i
SubbandToLevel3(int S) {
  if (S == 0)
    return v3i(0, 0, 0);
  /* handle level 0 which has only 1 subband (other levels have 7 subbands) */
  int Lvl = (S + 6) / 7;
  /* subtract all subbands on previous levels (except the subband 0);
  basically it reduces the case to the 2x2x2 case where subband 0 is in corner */
  S -= 7 * (Lvl - 1);
  /* bit 0 -> z axis offset; bit 1 -> y axis offset; bit 2 -> x axis offset
  we subtract from Lvl as it corresponds to the +x, +y, +z corner */
  return v3i(Lvl - !BitSet(S, 2), Lvl - !BitSet(S, 1), Lvl - !BitSet(S, 0));
}

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


