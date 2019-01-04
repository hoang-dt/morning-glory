#pragma once

#include "mg_bitops.h"
#include "mg_types.h"

namespace mg {

mg_ForceInline
extent::extent() = default;

mg_ForceInline
extent::extent(v3i Dims)
  : Pos(Stuff3Ints(v3i(0, 0, 0)))
  , Dims(Stuff3Ints(Dims)) {}

mg_ForceInline
extent::extent(v3i Pos, v3i Dims)
  : Pos(Stuff3Ints(Pos))
  , Dims(Stuff3Ints(Dims)) {}

mg_ForceInline
sub_volume::sub_volume() = default;
mg_ForceInline
sub_volume::sub_volume(volume Vol)
  : volume(Vol)
  , Extent(v3i(0, 0, 0), Extract3Ints(Vol.Dims)) {}

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

} // namespace mg
