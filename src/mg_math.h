#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

constexpr f64 Pi = 3.14159265358979323846;

/* Generate a power table for a particular base and type */
template <int N>
int (&Power(int Base))[N];

bool IsEven(i64 X);
bool IsOdd (i64 X);
v3i  IsEven(const v3i& P);
v3i  IsOdd (const v3i& P);

mg_T(t) int Exponent(t Val);

mg_TT(t, u = t) t Prod(const v3<u>& Vec);

// TODO: generalize the t parameter to u for the RHS
mg_T(t) v3<t> operator+(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator-(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator*(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator/(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator&(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator%(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator+(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator-(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator-(t Val, const v3<t>& Lhs);
mg_T(t) v3<t> operator*(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator/(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator&(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator%(const v3<t>& Lhs, t Val);
mg_T(t) bool  operator==(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator!=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator<=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator< (const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator> (const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator>=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_TT(t, u) v3<t> operator>>(const v3<t>& Lhs, const v3<u>& Rhs);
mg_TT(t, u) v3<t> operator>>(const v3<t>& Lhs, u Val);
mg_TT(t, u) v3<u> operator>>(u Val, const v3<t>& Rhs);
mg_TT(t, u) v3<t> operator<<(const v3<t>& Lhs, const v3<u>& Rhs);
mg_TT(t, u) v3<t> operator<<(const v3<t>& Lhs, u Val);
mg_TT(t, u) v3<u> operator<<(u Val, const v3<t>& Rhs);

mg_T(t) v3<t> Min(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> Max(const v3<t>& Lhs, const v3<t>& Rhs);

i8 Log2Floor(i64 Val);
i8 Log8Floor(i64 Val);

i64 Pow(i64 Base, int Exp);
i64 NextPow2(i64 Val);

int GeometricSum(int Base, int N);

} // namespace mg

#include "mg_math.inl"
