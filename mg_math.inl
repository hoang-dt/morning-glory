#pragma once

#include <math.h>
#include "mg_algorithm.h"
#include "mg_types.h"

namespace mg {

template <typename t>
bool IsEven(t x) {
  return (x & 1) == 0;
}

template <typename t>
bool IsOdd(t x) {
  return (x & 1) != 0;
}

template <typename t, int N>
const t (&Power(t Base))[N] {
  static t Table[N];
  Table[0] = 1;
  for (int I = 1; I < N; ++I)
    Table[I] = Table[I - 1] * Base;
  return Table;
}

template <typename t>
int Exponent(t Val) {
  if (Val > 0) {
    int E;
    frexp(Val, &E);
    /* clamp exponent in case Val is denormal */
    return Max(E, 1 - Traits<t>::ExponentBias);
  }
  return -Traits<t>::ExponentBias;
}

inline i64 XyzToI(v3i N, v3i P) {
  return i64(P.Z) * N.X * N.Y + i64(P.Y) * N.X + P.X;
}

inline v3i IToXyz(i64 I, v3i N) {
  i32 Z = I / (N.X * N.Y);
  i32 X = I % N.X;
  i32 Y = (I - i64(Z) * (N.X * N.Y)) / N.X;
  return v3i(X, Y, Z);
}

template <typename t, typename u>
t Prod(v3<u> Vec) {
  return t(Vec.X) * t(Vec.Y) * t(Vec.Z);
}

template <typename t>
v3<t> operator+(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X + Rhs.X, Lhs.Y + Rhs.Y, Lhs.Z + Rhs.Z };
}

template <typename t>
v3<t> operator+(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X + Val, Lhs.Y + Val, Lhs.Z + Val };
}

template <typename t>
v3<t> operator-(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X - Rhs.X, Lhs.Y - Rhs.Y, Lhs.Z - Rhs.Z };
}

template <typename t>
v3<t> operator-(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X - Val, Lhs.Y - Val, Lhs.Z - Val };
}

template <typename t>
v3<t> operator*(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X * Rhs.X, Lhs.Y * Rhs.Y, Lhs.Z * Rhs.Z };
}

template <typename t>
v3<t> operator*(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X * Val, Lhs.Y * Val, Lhs.Z * Val };
}

template <typename t>
v3<t> operator/(v3<t> Lhs, v3<t> Rhs) {
  return v3<t>{ Lhs.X / Rhs.X, Lhs.Y / Rhs.Y, Lhs.Z / Rhs.Z };
}

template <typename t>
v3<t> operator/(v3<t> Lhs, t Val) {
  return v3<t>{ Lhs.X / Val, Lhs.Y / Val, Lhs.Z / Val };
}

} // namespace mg
