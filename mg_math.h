#pragma once

namespace mg {

/* Generate a power table for a particular base and type */
template <typename t, int N>
t (&Power(t Base))[N] {
  static t Table[N];
  Table[0] = 1;
  for (int I = 1; I <= N; ++I)
    Table[I] = Table[I - 1] * Base;
  return Table;
}
static auto& Pow10 = Power<int, 9>(10);

} // namespace mg
