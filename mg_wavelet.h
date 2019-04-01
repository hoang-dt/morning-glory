#pragma once

#include "mg_common_types.h"
#include "mg_types.h"
#include "mg_volume.h"

// TODO: make these functions take in volume instead of raw pointers

namespace mg {

template <typename t> void ForwardLiftCdf53X(t* F, v3i N, v3i L);
template <typename t> void ForwardLiftCdf53Y(t* F, v3i N, v3i L);
template <typename t> void ForwardLiftCdf53Z(t* F, v3i N, v3i L);
template <typename t> void InverseLiftCdf53X(t* F, v3i N, v3i L);
template <typename t> void InverseLiftCdf53Y(t* F, v3i N, v3i L);
template <typename t> void InverseLiftCdf53Z(t* F, v3i N, v3i L);

template <typename t> void ForwardLiftExtrapolateCdf53X(t* F, v3i N, v3i NBig, v3i L);
template <typename t> void ForwardLiftExtrapolateCdf53Y(t* F, v3i N, v3i NBig, v3i L);
template <typename t> void ForwardLiftExtrapolateCdf53Z(t* F, v3i N, v3i NBig, v3i L);
template <typename t> void InverseLiftExtrapolateCdf53X(t* F, v3i N, v3i NBig, v3i L);
template <typename t> void InverseLiftExtrapolateCdf53Y(t* F, v3i N, v3i NBig, v3i L);
template <typename t> void InverseLiftExtrapolateCdf53Z(t* F, v3i N, v3i NBig, v3i L);

void Cdf53Forward(volume* Vol, int NLevels);
void Cdf53Inverse(volume* Vol, int NLevels);
void Cdf53ForwardExtrapolate(sub_volume* Vol);
void Cdf53InverseExtrapolate(sub_volume* Vol);

template <typename t>
struct dynamic_array;
struct extent;
void BuildSubbands(v3i N, int NLevels, dynamic_array<extent>* Subbands);

/* Utilities */

int LevelToSubband(v3i Level);

} // namespace mg

#include "mg_wavelet.inl"
