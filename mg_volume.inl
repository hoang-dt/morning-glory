#pragma once

#include "mg_bitops.h"
#include "mg_types.h"

namespace mg {

mg_ForceInline
block_bounds::block_bounds() = default;

mg_ForceInline
block_bounds::block_bounds(v3i SmallDims, v3i BigDims)
  : Pos(Stuff3Ints(v3i(0, 0, 0)))
  , SmallDims(Stuff3Ints(SmallDims))
  , BigDims(Stuff3Ints(BigDims)) {}

mg_ForceInline
block_bounds::block_bounds(v3i Pos, v3i SmallDims, v3i BigDims)
  : Pos(Stuff3Ints(Pos))
  , SmallDims(Stuff3Ints(SmallDims))
  , BigDims(Stuff3Ints(BigDims)) {}

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
