#pragma once

#include "mg_common_types.h"
#include "mg_types.h"

namespace mg {

template <typename t>
void ForwardLiftCdf53X(t* F, v3i N, v3i L);
template <typename t>
void ForwardLiftCdf53Y(t* F, v3i N, v3i L);
template <typename t>
void ForwardLiftCdf53Z(t* F, v3i N, v3i L);

template <typename t>
void InverseLiftCdf53X(t* F, v3i N, v3i L);
template <typename t>
void InverseLiftCdf53Y(t* F, v3i N, v3i L);
template <typename t>
void InverseLiftCdf53Z(t* F, v3i N, v3i L);

void Cdf53Forward(f64* F, v3i Dimensions, int NLevels, data_type Type = data_type::float64);
void Cdf53Inverse(f64* F, v3i Dimensions, int NLevels, data_type Type = data_type::float64);

struct Block {
  i64 Pos;
  i64 Size;
};

template <typename t>
struct dynamic_array;
void BuildSubbands(int NDims, v3i N, int NLevels, dynamic_array<Block>* Subbands);

} // namespace mg

#include "mg_wavelet.inl"
