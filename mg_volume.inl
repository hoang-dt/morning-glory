#pragma once

#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common_types.h"
#include "mg_types.h"

namespace mg {


mg_ForceInline
extent::extent() = default;

mg_ForceInline
extent::extent(v3i Dims)
  : PosCompact(Stuff3Ints64(v3i(0, 0, 0)))
  , DimsCompact(Stuff3Ints64(Dims)) {}

mg_ForceInline
extent::extent(v3i Pos, v3i Dims)
  : PosCompact(Stuff3Ints64(Pos))
  , DimsCompact(Stuff3Ints64(Dims)) {}

mg_ForceInline
bool IsPoint(extent Ext) {
  v3i Dims = Extract3Ints64(Ext.DimsCompact);
  return Dims.X * Dims.Y * Dims.Z == 1;
}

mg_ForceInline
bool IsLine(extent Ext) {
  v3i Dims = Extract3Ints64(Ext.DimsCompact);
  return (Dims.X * Dims.Y == 1) || (Dims.Y * Dims.Z == 1) || (Dims.X * Dims.Z == 1);
}

mg_ForceInline
bool IsFace(extent Ext) {
  v3i Dims = Extract3Ints64(Ext.DimsCompact);
  return Dims.X == 1 || Dims.Y == 1 || Dims.Z == 1;
}

mg_ForceInline
v3i Pos(extent Ext) {
  return Extract3Ints64(Ext.PosCompact);
}

mg_ForceInline
v3i Dims(extent Ext) {
  return Extract3Ints64(Ext.DimsCompact);
}

mg_ForceInline
v3i BigDims(const volume& Vol) {
  return Extract3Ints64(Vol.DimsCompact);
}
mg_ForceInline
v3i SmallDims(const volume& Vol) {
  return Dims(Vol.Extent);
}

i64 Size(const volume& Vol) {
  return Prod<i64>(Extract3Ints64(Vol.Extent.DimsCompact));
}

template <typename t> mg_ForceInline
t& At(volume& Vol, i64 I) {
  mg_Assert(MatchTypes<t>(Vol.Type));
  mg_Assert(I < Prod<i64>(Dims(Vol.Extent)));
  v3i Pos = IToXyz(I, Dims(Vol.Extent)) + Pos(Vol.Extent);
  volume& Vol = (volume&)Vol;
  i64 J = XyzToI(Dims(Vol), Pos);
  mg_Assert(I < Prod<i64>(Extract3Ints64(Vol.DimsCompact)));
  t* Ptr = (t*)Vol.Buffer.Data;
  return Ptr[I];
}

template <typename t> mg_ForceInline
t At(const volume& Vol, i64 I) {
  mg_Assert(MatchTypes<t>(Vol.Type));
  mg_Assert(I < Prod<i64>(Dims(Vol.Extent)));
  v3i Pos = IToXyz(I, Dims(Vol.Extent)) + Pos(Vol.Extent);
  const volume& Vol = (const volume&)Vol;
  i64 J = XyzToI(Dims(Vol), Pos);
  mg_Assert(I < Prod<i64>(Extract3Ints64(Vol.DimsCompact)));
  const t* Ptr = (t*)Vol.Buffer.Data;
  return Ptr[I];
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

mg_ForceInline
int NumDims(v3i N) {
  return (N.X > 1) + (N.Y > 1) + (N.Z > 1);
}

} // namespace mg
