#pragma once

namespace mg {

template <typename t, int N>
t (&Power(t Base))[N] {
  static t Table[N];
  Table[0] = 1;
  for (int I = 1; I < N; ++I)
    Table[I] = Table[I - 1] * Base;
  return Table;
}

} // namespace mg
