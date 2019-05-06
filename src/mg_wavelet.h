#pragma once

#include "mg_common.h"
#include "mg_volume.h"

// TODO: make these functions take in volume instead of raw pointers

namespace mg {

/* Normal lifting which uses mirroring at the boundary */
mg_T(t) void FLiftCdf53X(t* F, v3i N, v3i L);
mg_T(t) void FLiftCdf53Y(t* F, v3i N, v3i L);
mg_T(t) void FLiftCdf53Z(t* F, v3i N, v3i L);
mg_T(t) void ILiftCdf53X(t* F, v3i N, v3i L);
mg_T(t) void ILiftCdf53Y(t* F, v3i N, v3i L);
mg_T(t) void ILiftCdf53Z(t* F, v3i N, v3i L);

/* Lifting with extrapolation */
mg_T(t) void FLiftExtCdf53X(t* F, v3i N, v3i NBig, v3i L);
mg_T(t) void FLiftExtCdf53Y(t* F, v3i N, v3i NBig, v3i L);
mg_T(t) void FLiftExtCdf53Z(t* F, v3i N, v3i NBig, v3i L);
mg_T(t) void ILiftExtCdf53X(t* F, v3i N, v3i NBig, v3i L);
mg_T(t) void ILiftExtCdf53Y(t* F, v3i N, v3i NBig, v3i L);
mg_T(t) void ILiftExtCdf53Z(t* F, v3i N, v3i NBig, v3i L);

void ForwardCdf53(volume* Vol, int NLevels);
void InverseCdf53(volume* Vol, int NLevels);
void ForwardCdf53Ext(volume* Vol);
void InverseCdf53Ext(volume* Vol);

mg_T(t) struct array;
void BuildSubbands(v3i N, int NLevels, array<grid<>>* Subbands);
void BuildSubbandsInPlace(v3i N, int NLevels, array<grid<>>* Subbands);

void FormSubbands(volume* Dst, volume Src, int NLevels);
int LevelToSubband(v3i Level);
v3i ExpandDomain(v3i N, int NLevels);

} // namespace mg

#include "mg_wavelet.inl"
