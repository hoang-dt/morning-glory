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
    return Max(E, 1 - Traits<t>::ExponentBias);
  }
  return -Traits<t>::ExponentBias;
}

mg_ForceInline
i64 XyzToI(v3i N, v3i P) {
  return i64(P.Z) * N.X * N.Y + i64(P.Y) * N.X + P.X;
}

mg_ForceInline
v3i IToXyz(i64 I, v3i N) {
  i32 Z = I / (N.X * N.Y);
  i32 X = I % N.X;
  i32 Y = (I - i64(Z) * (N.X * N.Y)) / N.X;
  return v3i(X, Y, Z);
}

template <typename t, typename u> mg_ForceInline
t Prod(v3<u> Vec) {
  return t(Vec.X) * t(Vec.Y) * t(Vec.Z);
}

template <typename t> mg_ForceInline
v3<t> operator+(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X + Rhs.X, Lhs.Y + Rhs.Y, Lhs.Z + Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator+(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X + Val, Lhs.Y + Val, Lhs.Z + Val };
}

template <typename t> mg_ForceInline
v3<t> operator-(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X - Rhs.X, Lhs.Y - Rhs.Y, Lhs.Z - Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator-(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X - Val, Lhs.Y - Val, Lhs.Z - Val };
}

template <typename t> mg_ForceInline
v3<t> operator*(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X * Rhs.X, Lhs.Y * Rhs.Y, Lhs.Z * Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator*(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X * Val, Lhs.Y * Val, Lhs.Z * Val };
}

template <typename t> mg_ForceInline
v3<t> operator/(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X / Rhs.X, Lhs.Y / Rhs.Y, Lhs.Z / Rhs.Z };
}

template <typename t> mg_ForceInline
v3<t> operator/(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X / Val, Lhs.Y / Val, Lhs.Z / Val };
}

template <typename t> mg_ForceInline
bool operator==(v3<t> Lhs, v3<t> Rhs) {
  return Lhs.X == Rhs.X && Lhs.Y == Rhs.Y && Lhs.Z == Rhs.Z;
}

template <typename t> mg_ForceInline
bool operator<=(v3<t> Lhs, v3<t> Rhs) {
  return Lhs.X <= Rhs.X && Lhs.Y <= Rhs.Y && Lhs.Z <= Rhs.Z;
}

mg_ForceInline
i8 Log2Floor(int Val) {
  mg_Assert(Val > 0);
  return BitScanReverse((u32)Val);
}

mg_ForceInline
i8 Log8Floor(int Val) {
  mg_Assert(Val > 0);
  return Log2Floor(Val) / 3;
}

} // namespace mg
