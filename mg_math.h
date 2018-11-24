#pragma once

namespace mg {

/* Generate a power table for a particular base and type */
template <typename t, int N>
t (&Power(t Base))[N];

} // namespace mg

#include "mg_math.inl"

namespace mg {

/* Table for powers of 10 */
static auto& Pow10 = Power<int, 9>(10);

} // namespace mg
