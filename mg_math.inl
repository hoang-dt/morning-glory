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
    /* clamp exponent in case x is denormal */
    return Max(E, 1 - Traits<t>::ExponentBias);
  }
  return -Traits<t>::ExponentBias;
}

} // namespace mg
