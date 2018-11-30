#pragma once

#include "mg_common_types.h"
#include "mg_types.h"

namespace mg {

template <typename t>
void ForwardLiftCdf53X(t* F, v3l N, v3l L);
template <typename t>
void ForwardLiftCdf53Y(t* F, v3l N, v3l L);
template <typename t>
void ForwardLiftCdf53Z(t* F, v3l N, v3l L);

template <typename t>
void InverseLiftCdf53X(t* F, v3l N, v3l L);
template <typename t>
void InverseLiftCdf53Y(t* F, v3l N, v3l L);
template <typename t>
void InverseLiftCdf53Z(t* F, v3l N, v3l L);

void Cdf53Forward(f64* F, v3l Dimensions, int NLevels, data_type Type = data_type::float64);
void Cdf53Inverse(f64* F, v3l Dimensions, int NLevels, data_type Type = data_type::float64);

}

#include "mg_wavelet.inl"
