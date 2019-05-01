#pragma once

#include <math.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_macros.h"
#include "mg_types.h"

namespace mg {

mg_ForceInline
bool IsEven(int X) {
  return (X & 1) == 0;
}
mg_ForceInline
bool IsOdd(int X) {
  return (X & 1) != 0;
}
mg_ForceInline
v3i IsEven(const v3i& P) {
  return v3i(IsEven(P.X), IsEven(P.Y), IsEven(P.Z));
}
mg_ForceInline
v3i IsOdd(const v3i& P) {
  return v3i(IsOdd(P.X), IsOdd(P.Y), IsOdd(P.Z));
}

mg_ForceInline
bool IsPow2(int X) {
  mg_Assert(X > 0);
  return X && !(X & (X - 1));
}

template <int N>
const int (&Power(int Base))[N] {
  static int Table[N];
  Table[0] = 1;
  for (int I = 1; I < N; ++I)
    Table[I] = Table[I - 1] * Base;
  return Table;
}

template <typename t> mg_ForceInline
int Exponent(t Val) {
  if (Val > 0) {
    int E;
    frexp(Val, &E);
    /* clamp exponent in case Val is denormal */
    return Max(E, 1 - Traits<t>::ExpBias);
  }
  return -Traits<t>::ExpBias;
}

template <typename t = int, typename u> mg_ForceInline
t Prod(const v3<u>& Vec) {
  return t(Vec.X) * t(Vec.Y) * t(Vec.Z);
}

template <typename t> mg_ForceInline
v3<t> operator+(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>{ Lhs.X + Rhs.X, Lhs.Y + Rhs.Y, Lhs.Z + Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator+(const v3<t>& Lhs, t Val) {
  return v3<t>{ Lhs.X + Val, Lhs.Y + Val, Lhs.Z + Val };
}

template <typename t> mg_ForceInline
v3<t> operator-(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>{ Lhs.X - Rhs.X, Lhs.Y - Rhs.Y, Lhs.Z - Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator-(const v3<t>& Lhs, t Val) {
  return v3<t>{ Lhs.X - Val, Lhs.Y - Val, Lhs.Z - Val };
}

template <typename t> mg_ForceInline
v3<t> operator*(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>{ Lhs.X * Rhs.X, Lhs.Y * Rhs.Y, Lhs.Z * Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator*(const v3<t>& Lhs, t Val) {
  return v3<t>{ Lhs.X * Val, Lhs.Y * Val, Lhs.Z * Val };
}

template <typename t> mg_ForceInline
v3<t> operator/(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>{ Lhs.X / Rhs.X, Lhs.Y / Rhs.Y, Lhs.Z / Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator/(const v3<t>& Lhs, t Val) {
  return v3<t>{ Lhs.X / Val, Lhs.Y / Val, Lhs.Z / Val };
}

template <typename t> mg_ForceInline
bool operator==(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X == Rhs.X && Lhs.Y == Rhs.Y && Lhs.Z == Rhs.Z;
}

template <typename t> mg_ForceInline
bool operator<=(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X <= Rhs.X && Lhs.Y <= Rhs.Y && Lhs.Z <= Rhs.Z;
}

template <typename t> mg_ForceInline
bool operator<(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X < Rhs.X && Lhs.Y < Rhs.Y && Lhs.Z < Rhs.Z;
}

template <typename t> mg_ForceInline
bool operator>=(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X >= Rhs.X && Lhs.Y >= Rhs.Y && Lhs.Z >= Rhs.Z;
}

template <typename t> mg_ForceInline
v3<t> Min(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Min(Lhs.X, Rhs.X), Min(Lhs.Y, Rhs.Y), Min(Lhs.Z, Rhs.Z));
}

template <typename t> mg_ForceInline
v3<t> Max(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Max(Lhs.X, Rhs.X), Max(Lhs.Y, Rhs.Y), Max(Lhs.Z, Rhs.Z));
}

mg_ForceInline
i8 Log2Floor(int Val) {
  mg_Assert(Val > 0);
  return Msb((u32)Val);
}

mg_ForceInline
i8 Log8Floor(int Val) {
  mg_Assert(Val > 0);
  return Log2Floor(Val) / 3;
}

} // namespace mg
