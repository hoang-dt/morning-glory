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
mg_T(t) void FLiftCdf53X(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap);
mg_T(t) void FLiftCdf53Y(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap);
mg_T(t) void FLiftCdf53Z(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap);
mg_T(t) void ILiftCdf53X(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap);
mg_T(t) void ILiftCdf53Y(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap);
mg_T(t) void ILiftCdf53Z(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap);

/* Lifting with extrapolation */
mg_T(t) void FLiftExtCdf53X(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void FLiftExtCdf53Y(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void FLiftExtCdf53Z(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void ILiftExtCdf53X(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void ILiftExtCdf53Y(t* F, const v3i& N, const v3i& NBig, const v3i& L);
mg_T(t) void ILiftExtCdf53Z(t* F, const v3i& N, const v3i& NBig, const v3i& L);

void ForwardCdf53(grid_volume* Vol, int NLevels);
void InverseCdf53(grid_volume* Vol, int NLEvels);
void ForwardCdf53Old(volume* Vol, int NLevels);
void InverseCdf53Old(volume* Vol, int NLevels);
void ForwardCdf53Ext(grid_volume* Grid);
void InverseCdf53Ext(grid_volume* Grid);

mg_T(t) struct array;
void BuildSubbands(const v3i& N, int NLevels, array<grid>* Subbands);
void BuildSubbandsInPlace(const v3i& N, int NLevels, array<grid>* Subbands);
/* Copy samples from Src so that in Dst, samples are organized into subbands */
void FormSubbands(grid_volume Dst, grid_volume Src, int NLevels);
/* Assume the wavelet transform is done in X, then Y, then Z */
int LevelToSubband(const v3i& Level);
v3i ExpandDomain(const v3i& N, int NLevels);

} // namespace mg

#include "mg_wavelet.inl"
