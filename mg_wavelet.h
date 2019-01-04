#pragma once

#include "mg_common_types.h"
#include "mg_types.h"

// TODO: make these functions take in volume instead of raw pointers

namespace mg {

template <typename t> void ForwardLiftCdf53X(t* F, v3i N, v3i L);
template <typename t> void ForwardLiftCdf53Y(t* F, v3i N, v3i L);
template <typename t> void ForwardLiftCdf53Z(t* F, v3i N, v3i L);
template <typename t> void InverseLiftCdf53X(t* F, v3i N, v3i L);
template <typename t> void InverseLiftCdf53Y(t* F, v3i N, v3i L);
template <typename t> void InverseLiftCdf53Z(t* F, v3i N, v3i L);

template <typename t> void ForwardLiftExtrapolateCdf53X(t* F, v3i N, v3i L);
template <typename t> void ForwardLiftExtrapolateCdf53Y(t* F, v3i N, v3i L);
template <typename t> void ForwardLiftExtrapolateCdf53Z(t* F, v3i N, v3i L);
template <typename t> void InverseLiftExtrapolateCdf53X(t* F, v3i N, v3i L);
template <typename t> void InverseLiftExtrapolateCdf53Y(t* F, v3i N, v3i L);
template <typename t> void InverseLiftExtrapolateCdf53Z(t* F, v3i N, v3i L);

void Cdf53Forward(f64* F, v3i Dimensions, int NLevels,
                  data_type Type = data_type::float64);
void Cdf53Inverse(f64* F, v3i Dimensions, int NLevels,
                  data_type Type = data_type::float64);
void Cdf53ForwardExtrapolate(f64* F, v3i Dimensions, int NLevels,
                             data_type Type = data_type::float64);
void Cdf53InverseExtrapolate(f64* F, v3i Dimensions, int NLevels,
                             data_type Type = data_type::float64);

template <typename t>
struct dynamic_array;
struct block_bounds;
void BuildSubbands(int NDims, v3i N, int NLevels, dynamic_array<block_bounds>* Subbands);

} // namespace mg

#include "mg_wavelet.inl"
