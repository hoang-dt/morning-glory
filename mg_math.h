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

i64 XyzToI(v3l N, v3l P);
v3l IToXyz(i64 i, v3l N);

template <typename t>
t Prod(v3<t> Vec);

template <typename t>
v3<t> operator*(v3<t> Lhs, v3<t> Rhs);

} // namespace mg
