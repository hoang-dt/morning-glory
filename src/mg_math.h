#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

/* Generate a power table for a particular base and type */
template <int N>
int (&Power(int Base))[N];

bool IsEven(int X);
bool IsOdd (int X);
v3i  IsEven(const v3i& P);
v3i  IsOdd (const v3i& P);

mg_T(t) int Exponent(t Val);

mg_T2(t, u = t) t Prod(const v3<u>& Vec);

mg_T(t) v3<t> operator+(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator-(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator*(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator/(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> operator+(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator-(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator*(const v3<t>& Lhs, t Val);
mg_T(t) v3<t> operator/(const v3<t>& Lhs, t Val);
mg_T(t) bool  operator==(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator!=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator<=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator< (const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) bool  operator>=(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> Min(const v3<t>& Lhs, const v3<t>& Rhs);
mg_T(t) v3<t> Max(const v3<t>& Lhs, const v3<t>& Rhs);

i8 Log2Floor(int Val);
i8 Log8Floor(int Val);

int Pow(int Base, int Exp);
int NextPow2(int Val);

int GeometricSum(int Base, int N);

} // namespace mg

#include "mg_math.inl"

namespace mg {

static auto& Pow10 = Power<9>(10);
static auto& Pow8  = Power<10>(8);

} // namespace mg
