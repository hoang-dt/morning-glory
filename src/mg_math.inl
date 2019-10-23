#pragma once

#include <math.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common.h"

namespace mg {

mg_Inline bool
IsEven(i64 X) { return (X & 1) == 0; }
mg_Inline bool
IsOdd(i64 X) { return (X & 1) != 0; }
mg_Inline v3i
IsEven(const v3i& P) { return v3i(IsEven(P.X), IsEven(P.Y), IsEven(P.Z)); }
mg_Inline v3i
IsOdd(const v3i& P) { return v3i(IsOdd(P.X), IsOdd(P.Y), IsOdd(P.Z)); }

mg_Ti(t) bool
IsBetween(t Val, t A, t B) { return (A <= Val && Val <= B) || (B <= Val && Val <= A); }

mg_Inline bool
IsPow2(int X) { mg_Assert(X > 0); return X && !(X & (X - 1)); }

mg_Inline constexpr int
LogFloor(i64 Base, i64 Val) {
  int Log = 0;
  i64 S = Base;
  while (S <= Val) {
    ++Log;
    S *= Base;
  }
  return Log;
}

mg_TI(t, N)
struct pow {
  static inline const stack_array<t, LogFloor(N, traits<t>::Max)> Table = []() {
    stack_array<t, LogFloor(N, traits<t>::Max)> Result;
    t Base = N;
    t Pow = 1;
    for (int I = 0; I < Size(Result); ++I) {
      Result[I] = Pow;
      Pow *= Base;
    }
    return Result;
  }();
  t operator[](int I) const { return Table[I]; }
};

mg_Ti(t) int
Exponent(t Val) {
  if (Val > 0) {
    int E;
    frexp(Val, &E);
    /* clamp exponent in case Val is denormal */
    return Max(E, 1 - traits<t>::ExpBias);
  }
  return -traits<t>::ExpBias;
}

mg_Ti(t) v2<t>
operator+(const v2<t>& Lhs, const v2<t>& Rhs) {
  return v2<t>(Lhs.X + Rhs.X, Lhs.Y + Rhs.Y);
}
mg_Ti(t) v2<t> operator+(const v2<t>& Lhs, t Val) {
  return v2<t>(Lhs.X + Val, Lhs.Y + Val);
}
mg_Ti(t) v2<t>
operator-(const v2<t>& Lhs, const v2<t>& Rhs) {
  return v2<t>(Lhs.X - Rhs.X, Lhs.Y - Rhs.Y);
}
mg_Ti(t) v2<t> operator-(const v2<t>& Lhs, t Val) {
  return v2<t>(Lhs.X - Val, Lhs.Y - Val);
}
mg_Ti(t) v2<t>
operator*(const v2<t>& Lhs, const v2<t>& Rhs) {
  return v2<t>(Lhs.X * Rhs.X, Lhs.Y * Rhs.Y);
}
mg_Ti(t) v2<t> operator*(const v2<t>& Lhs, t Val) {
  return v2<t>(Lhs.X * Val, Lhs.Y * Val);
}
mg_Ti(t) v2<t>
operator/(const v2<t>& Lhs, const v2<t>& Rhs) {
  return v2<t>(Lhs.X / Rhs.X, Lhs.Y / Rhs.Y);
}
mg_Ti(t) v2<t> operator/(const v2<t>& Lhs, t Val) {
  return v2<t>(Lhs.X / Val, Lhs.Y / Val);
}
mg_Ti(t) v2<t>
Min(const v2<t>& Lhs, const v2<t>& Rhs) {
  return v2<t>(Min(Lhs.X, Rhs.X), Min(Lhs.Y, Rhs.Y));
}
mg_Ti(t) v2<t>
Max(const v2<t>& Lhs, const v2<t>& Rhs) {
  return v2<t>(Max(Lhs.X, Rhs.X), Max(Lhs.Y, Rhs.Y));
}

mg_TTi(t = int, u) t
Prod(const v2<u>& Vec) { return t(Vec.X) * t(Vec.Y); }

mg_TTi(t = int, u) t
Prod(const v3<u>& Vec) { return t(Vec.X) * t(Vec.Y) * t(Vec.Z); }

mg_Ti(t) v3<t>
operator+(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Lhs.X + Rhs.X, Lhs.Y + Rhs.Y, Lhs.Z + Rhs.Z);
}
mg_Ti(t) v3<t>
operator+(const v3<t>& Lhs, t Val) {
  return v3<t>(Lhs.X + Val, Lhs.Y + Val, Lhs.Z + Val);
}
mg_Ti(t) v3<t>
operator-(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Lhs.X - Rhs.X, Lhs.Y - Rhs.Y, Lhs.Z - Rhs.Z);
}
mg_Ti(t) v3<t>
operator-(const v3<t>& Lhs, t Val) {
  return v3<t>(Lhs.X - Val, Lhs.Y - Val, Lhs.Z - Val);
}
mg_Ti(t) v3<t>
operator-(t Val, const v3<t>& Lhs) {
  return v3<t>(Val - Lhs.X, Val - Lhs.Y, Val - Lhs.Z);
}
mg_Ti(t) v3<t>
operator*(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Lhs.X * Rhs.X, Lhs.Y * Rhs.Y, Lhs.Z * Rhs.Z);
}
mg_Ti(t) v3<t>
operator*(const v3<t>& Lhs, t Val) {
  return v3<t>(Lhs.X * Val, Lhs.Y * Val, Lhs.Z * Val);
}
mg_Ti(t) v3<t>
operator/(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Lhs.X / Rhs.X, Lhs.Y / Rhs.Y, Lhs.Z / Rhs.Z);
}
mg_Ti(t) v3<t>
operator/(const v3<t>& Lhs, t Val) {
  return v3<t>(Lhs.X / Val, Lhs.Y / Val, Lhs.Z / Val);
}
mg_Ti(t) v3<t>
operator&(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Lhs.X & Rhs.X, Lhs.Y & Rhs.Y, Lhs.Z & Rhs.Z);
}
mg_Ti(t) v3<t>
operator&(const v3<t>& Lhs, t Val) {
  return v3<t>(Lhs.X & Val, Lhs.Y & Val, Lhs.Z & Val);
}
mg_Ti(t) v3<t> 
operator%(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Lhs.X % Rhs.X, Lhs.Y % Rhs.Y, Lhs.Z % Rhs.Z);
}
mg_Ti(t) v3<t> 
operator%(const v3<t>& Lhs, t Val) {
  return v3<t>(Lhs.X % Val, Lhs.Y % Val, Lhs.Z % Val);
}
mg_Ti(t) bool
operator==(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X == Rhs.X && Lhs.Y == Rhs.Y && Lhs.Z == Rhs.Z;
}
mg_Ti(t) bool
operator!=(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X != Rhs.X || Lhs.Y != Rhs.Y || Lhs.Z != Rhs.Z;
}
mg_Ti(t) bool
operator!=(const v3<t>& Lhs, t Val) {
  return Lhs.X != Val || Lhs.Y != Val || Lhs.Z != Val;
}
mg_Ti(t) bool
operator<=(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X <= Rhs.X && Lhs.Y <= Rhs.Y && Lhs.Z <= Rhs.Z;
}
mg_Ti(t) bool
operator<(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X < Rhs.X && Lhs.Y < Rhs.Y && Lhs.Z < Rhs.Z;
}
mg_Ti(t) bool
operator>(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X > Rhs.X && Lhs.Y > Rhs.Y && Lhs.Z > Rhs.Z;
}
mg_Ti(t) bool
operator>=(const v3<t>& Lhs, const v3<t>& Rhs) {
  return Lhs.X >= Rhs.X && Lhs.Y >= Rhs.Y && Lhs.Z >= Rhs.Z;
}
mg_TTi(t, u) v3<t>
operator>>(const v3<t>& Lhs, const v3<u>& Rhs) {
  return v3<t>(Lhs.X >> Rhs.X, Lhs.Y >> Rhs.Y, Lhs.Z >> Rhs.Z);
}
mg_TTi(t, u) v3<u>
operator>>(u Val, const v3<t>& Lhs) {
  return v3<u>(Val >> Lhs.X, Val >> Lhs.Y, Val >> Lhs.Z);
}
mg_TTi(t, u) v3<t>
operator>>(const v3<t>& Lhs, u Val) {
  return v3<t>(Lhs.X >> Val, Lhs.Y >> Val, Lhs.Z >> Val);
}
mg_TTi(t, u) v3<t>
operator<<(const v3<t>& Lhs, const v3<u>& Rhs) {
  return v3<t>(Lhs.X << Rhs.X, Lhs.Y << Rhs.Y, Lhs.Z << Rhs.Z);
}
mg_TTi(t, u) v3<t>
operator<<(const v3<t>& Lhs, u Val) {
  return v3<t>(Lhs.X << Val, Lhs.Y << Val, Lhs.Z << Val);
}
mg_TTi(t, u) v3<u>
operator<<(u Val, const v3<t>& Lhs) {
  return v3<u>(Val << Lhs.X, Val << Lhs.Y, Val << Lhs.Z);
}
mg_Ti(t) v3<t>
Min(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Min(Lhs.X, Rhs.X), Min(Lhs.Y, Rhs.Y), Min(Lhs.Z, Rhs.Z));
}
mg_Ti(t) v3<t>
Max(const v3<t>& Lhs, const v3<t>& Rhs) {
  return v3<t>(Max(Lhs.X, Rhs.X), Max(Lhs.Y, Rhs.Y), Max(Lhs.Z, Rhs.Z));
}

mg_Inline i8
Log2Floor(i64 Val) {
  mg_Assert(Val > 0);
  return Msb((u64)Val);
}

mg_Inline i8
Log8Floor(i64 Val) {
  mg_Assert(Val > 0);
  return Log2Floor(Val) / 3;
}

mg_Inline int
GeometricSum(int Base, int N) {
  mg_Assert(N >= 0);
  return (Pow(Base, N + 1) - 1) / (Base - 1);
}

// TODO: when n is already a power of two plus one, do not increase n
mg_Inline i64
NextPow2(i64 Val) {
  mg_Assert(Val >= 0);
  if (Val == 0)
    return 1;
  return 1 << (Msb((u64)(Val - 1)) + 1);
}

mg_Inline i64
Pow(i64 Base, int Exp) {
  mg_Assert(Exp >= 0);
  i64 Result = 1;
  for (int I = 0; I < Exp; ++I)
    Result *= Base;
  return Result;
}

mg_Ti(t) t
Lerp(t V1, t V2, f64 T) {
  mg_Assert(0 <= T && T <= 1);
  return V1 + (V2 - V1) * T;
}

} // namespace mg
