#pragma once

#include "mg_types.h"

namespace mg {

template <typename t>
void FLiftCdf53X(t* F, v3l N, v3l L);
template <typename t>
void FLiftCdf53Y(t* F, v3l N, v3l L);
template <typename t>
void FLiftCdf53Z(t* F, v3l N, v3l L);

template <typename t>
void ILiftCdf53X(t* F, v3l N, v3l L);
template <typename t>
void ILiftCdf53Y(t* F, v3l N, v3l L);
template <typename t>
void ILiftCdf53Z(t* F, v3l N, v3l L);

}

#include "mg_wavelet.inl"
