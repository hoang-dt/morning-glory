#pragma once

#include "mg_types.h"

namespace mg {

/* Generate a power table for a particular base and type */
template <int N>
const int (&Power(int Base))[N];

bool IsEven(int X);
bool IsOdd(int X);
v3i IsEven(v3i P);
v3i IsOdd(v3i P);

template <typename t>
int Exponent(t Val);

} // namespace mg

#include "mg_math.inl"

namespace mg {

/* Table for powers of 10 */
static auto& Pow10 = Power<9>(10);
static auto& Pow8  = Power<10>(8);

template <typename t, typename u = t> t Prod(const v3<u>& Vec);

template <typename t> v3<t> operator+(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> v3<t> operator-(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> v3<t> operator*(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> v3<t> operator/(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> v3<t> operator+(const v3<t>& Lhs, t Val);
template <typename t> v3<t> operator-(const v3<t>& Lhs, t Val);
template <typename t> v3<t> operator*(const v3<t>& Lhs, t Val);
template <typename t> v3<t> operator/(const v3<t>& Lhs, t Val);
template <typename t> bool operator==(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> bool operator<=(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> bool operator<(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> bool operator>=(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> v3<t> Min(const v3<t>& Lhs, const v3<t>& Rhs);
template <typename t> v3<t> Max(const v3<t>& Lhs, const v3<t>& Rhs);

/* Floor of the log2 of Val */
i8 Log2Floor(int Val);
i8 Log8Floor(int Val);

/* Compute Base^Exp (Exp >= 0) */
int Pow(int Base, int Exp);
/* Return the next power of two that is >= the input */
int NextPow2(int Val);

/* Compute Base^0 + Base^1 + ... + Base^N */
int GeometricSum(int Base, int N);

} // namespace mg
