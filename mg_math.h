#pragma once

#include "mg_types.h"

namespace mg {

/* Generate a power table for a particular base and type */
template <typename t, int N>
const t (&Power(t Base))[N];

template <typename t>
bool IsEven(t x);
template <typename t>
bool IsOdd(t x);

template <typename t>
int Exponent(t Val);

} // namespace mg

#include "mg_math.inl"

namespace mg {

/* Table for powers of 10 */
static auto& Pow10 = Power<int, 9>(10);

} // namespace mg
