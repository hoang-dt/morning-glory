#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

/* Generate a power table for a particular base and type */
template <int N>
int (&Power(int Base))[N];

bool
IsEven(int X);
bool
IsOdd(int X);
v3i
IsEven(const v3i& P);
v3i
IsOdd(const v3i& P);

mg_T(t) int
Exponent(t Val);

template <typename t, typename u = t> t Prod(const v3<u>& Vec);

mg_T(t) v3<t>
operator+(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t>
operator-(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t>
operator*(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t>
operator/(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t>
operator+(const v3<t>& Lhs, t Val);
mg_T(t) v3<t>
operator-(const v3<t>& Lhs, t Val);
mg_T(t) v3<t>
operator*(const v3<t>& Lhs, t Val);
mg_T(t) v3<t>
operator/(const v3<t>& Lhs, t Val);
mg_T(t) bool
operator==(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool
operator<=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool
operator<(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool
operator>=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t>
Min(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t>
Max(const v3<t>& Lhs, const v3<t>& Rhs);

i8 /* Floor of the log2 of Val */
Log2Floor(int Val);
i8 /* Floor of the log8 of Val */
Log8Floor(int Val);

int /* Compute Base^Exp (Exp >= 0) */
Pow(int Base, int Exp);

int /* Return the next power of two that is >= the input */
NextPow2(int Val);

int /* Compute Base^0 + Base^1 + ... + Base^N */
GeometricSum(int Base, int N);

} // namespace mg

#include "mg_math.inl"

namespace mg {
static auto& Pow10 = Power<9>(10);
static auto& Pow8  = Power<10>(8);
} // namespace mg
