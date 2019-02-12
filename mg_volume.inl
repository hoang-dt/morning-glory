#pragma once

#include "mg_bitops.h"
#include "mg_types.h"

namespace mg {

mg_ForceInline extent::extent() = default;

mg_ForceInline extent::extent(v3i Dims)
  : PosCompact(Stuff3Ints(v3i(0, 0, 0)))
  , DimsCompact(Stuff3Ints(Dims)) {}

mg_ForceInline extent::extent(v3i Pos, v3i Dims)
  : PosCompact(Stuff3Ints(Pos))
  , DimsCompact(Stuff3Ints(Dims)) {}

mg_ForceInline bool IsPoint(extent Ext) {
  v3i Dims = Extract3Ints(Ext.DimsCompact);
  return Dims.X * Dims.Y * Dims.Z == 1;
}

mg_ForceInline bool IsLine(extent Ext) {
  v3i Dims = Extract3Ints(Ext.DimsCompact);
  return (Dims.X * Dims.Y == 1) || (Dims.Y * Dims.Z == 1) || (Dims.X * Dims.Z == 1);
}

mg_ForceInline bool IsFace(extent Ext) {
  v3i Dims = Extract3Ints(Ext.DimsCompact);
  return Dims.X == 1 || Dims.Y == 1 || Dims.Z == 1;
}

mg_ForceInline sub_volume::sub_volume() = default;
mg_ForceInline sub_volume::sub_volume(volume Vol)
  : volume(Vol)
  , Extent(v3i(0, 0, 0), Extract3Ints(Vol.DimsCompact)) {}

mg_ForceInline i64 XyzToI(v3i N, v3i P) {
  return i64(P.Z) * N.X * N.Y + i64(P.Y) * N.X + P.X;
}

mg_ForceInline v3i IToXyz(i64 I, v3i N) {
  i32 Z = I / (N.X * N.Y);
  i32 X = I % N.X;
  i32 Y = (I - i64(Z) * (N.X * N.Y)) / N.X;
  return v3i(X, Y, Z);
}

} // namespace mg
