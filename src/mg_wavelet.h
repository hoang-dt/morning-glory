#pragma once

#include "mg_common.h"
#include "mg_volume.h"

// TODO: make these functions take in volume instead of raw pointers

namespace mg {

/* Normal lifting which uses mirroring at the boundary */
mg_T(t) void FLiftCdf53OldX(t* F, const v3i& N, const v3i& L);
mg_T(t) void FLiftCdf53OldY(t* F, const v3i& N, const v3i& L);
mg_T(t) void FLiftCdf53OldZ(t* F, const v3i& N, const v3i& L);
mg_T(t) void ILiftCdf53OldX(t* F, const v3i& N, const v3i& L);
mg_T(t) void ILiftCdf53OldY(t* F, const v3i& N, const v3i& L);
mg_T(t) void ILiftCdf53OldZ(t* F, const v3i& N, const v3i& L);

/* New set of lifting functions */
enum lift_option { Normal, PartialUpdateLast, NoUpdateLast };
mg_T(t) void FLiftCdf53X(const grid& Grid, const v3i& M, lift_option Opt, volume* Vol);
mg_T(t) void FLiftCdf53Y(const grid& Grid, const v3i& M, lift_option Opt, volume* Vol);
mg_T(t) void FLiftCdf53Z(const grid& Grid, const v3i& M, lift_option Opt, volume* Vol);
mg_T(t) void ILiftCdf53X(const grid& Grid, const v3i& M, lift_option Opt, volume* Vol);
mg_T(t) void ILiftCdf53Y(const grid& Grid, const v3i& M, lift_option Opt, volume* Vol);
mg_T(t) void ILiftCdf53Z(const grid& Grid, const v3i& M, lift_option Opt, volume* Vol);

/* Lifting with extrapolation */
mg_T(t) void FLiftExtCdf53X(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void FLiftExtCdf53Y(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void FLiftExtCdf53Z(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void ILiftExtCdf53X(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void ILiftExtCdf53Y(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void ILiftExtCdf53Z(t* F, const v3i& N, const v3i& NBig, const v3i& L);

#define mg_Cdf53TileDebug

void ForwardCdf53Tile(int NLvls, const v3i& TDims3, const volume& Vol
#if defined(mg_Cdf53TileDebug)
  , volume* OutVol
#endif
);
void ForwardCdf53(const extent& Ext, int NLevels, volume* Vol);
void InverseCdf53(const extent& Ext, int NLevels, volume* Vol);
void ForwardCdf53Old(volume* Vol, int NLevels);
void InverseCdf53Old(volume* Vol, int NLevels);
void ForwardCdf53Ext(const extent& Ext, volume* Vol);
void InverseCdf53Ext(const extent& Ext, volume* Vol);

mg_T(t) struct array;
void BuildSubbands(const v3i& N, int NLevels, array<extent>* Subbands);
void BuildSubbandsInPlace(const v3i& N, int NLevels, array<grid>* Subbands);
/* Copy samples from Src so that in Dst, samples are organized into subbands */
void FormSubbands(int NLevels, const grid& SrcGrid, const volume& SrcVol,
                  const grid& DstGrid, volume* DstVol);
/* Assume the wavelet transform is done in X, then Y, then Z */
int LevelToSubband(const v3i& Level);
v3i ExpandDomain(const v3i& N, int NLevels);

v2i SubbandToLevel2(int S);
v3i SubbandToLevel3(int S);

} // namespace mg

#include "mg_wavelet.inl"
