#pragma once

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

} // namespace mg
