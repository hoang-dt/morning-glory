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
  , DimsCompact(Stuff3Ints64(Dims))
  , StrideCompact(Stuff3Ints64(v3i(1, 1, 1))) {}

mg_ForceInline
extent::extent(v3i Pos, v3i Dims)
  : PosCompact(Stuff3Ints64(Pos))
  , DimsCompact(Stuff3Ints64(Dims))
  , StrideCompact(Stuff3Ints64(v3i(1, 1, 1))) {}

mg_ForceInline
extent::extent(v3i Pos, v3i Dims, v3i Stride)
  : PosCompact(Stuff3Ints64(Pos))
  , DimsCompact(Stuff3Ints64(Dims))
  , StrideCompact(Stuff3Ints64(Stride)) {}

//mg_ForceInline
//bool IsPoint(extent Ext) {
//  v3i MyDims = Dims(Ext);
//  return Dims.X == 1 && Dims.Y == 1 && Dims.Z == 1;
//}
//
//mg_ForceInline
//bool IsLine(extent Ext) {
//  v3i MyDims = Dims(Ext);
//  return !IsPoint(Ext) && ((MyDims.X * MyDims.Y == 1) ||
//                           (MyDims.Y * MyDims.Z == 1) ||
//                           (MyDims.X * MyDims.Z == 1));
//}
//
//mg_ForceInline
//bool IsFace(extent Ext) {
//  v3i ExtDims = Dims(Ext);
//  return !IsLine(Ext) && (Dims.X == 1 || Dims.Y == 1 || Dims.Z == 1);
//}

mg_ForceInline
v3i Pos(extent Ext) {
  return Extract3Ints64(Ext.PosCompact);
}

mg_ForceInline
v3i Dims(extent Ext) {
  return Extract3Ints64(Ext.DimsCompact);
}

mg_ForceInline
v3i Stride(extent Ext) {
  return Extract3Ints64(Ext.StrideCompact);
}

mg_ForceInline
v3i BigDims(const volume& Vol) {
  return Extract3Ints64(Vol.DimsCompact);
}
mg_ForceInline
v3i SmallDims(const volume& Vol) {
  return Dims(Vol.Extent);
}

mg_ForceInline
i64 Size(const volume& Vol) {
  return Prod<i64>(SmallDims(Vol));
}

//template <typename t> mg_ForceInline
//t& At(volume& Vol, v3i MyPos) {
//  mg_Assert(MatchTypes<t>(Vol.Type));
//  auto MyPos = Pos(Vol.Extent) + MyPos * Stride(Vol.Extent);
//  mg_Assert(MyPos < SmallDims(Vol));
//  i64 I = XyzToI(BigDims(Vol), MyPos);
//  t* Ptr = (t*)Vol.Buffer.Data;
//  mg_Assert(I * (i64)sizeof(t) < Vol.Buffer.Bytes);
//  return Ptr[I];
//}
//
//template <typename t> mg_ForceInline
//t At(const volume& Vol, i64 I) {
//  mg_Assert(MatchTypes<t>(Vol.Type));
//  auto MyPos = Pos(Vol.Extent) + MyPos * Stride(Vol.Extent);
//  mg_Assert(MyPos < SmallDims(Vol));
//  i64 I = XyzToI(BigDims(Vol), MyPos);
//  const t* Ptr = (t*)Vol.Buffer.Data;
//  mg_Assert(I * (i64)sizeof(t) < Vol.Buffer.Bytes);
//  return Ptr[I];
//}

mg_ForceInline
i64 XyzToI(v3i N, v3i P) {
  return i64(P.Z) * N.X * N.Y + i64(P.Y) * N.X + P.X;
}

mg_ForceInline
v3i IToXyz(i64 I, v3i N) {
  i32 Z = i32(I / (N.X * N.Y));
  i32 XY = i32(I % (N.X * N.Y));
  return v3i(XY % N.X, XY / N.X, Z);
}

mg_ForceInline
int NumDims(v3i N) {
  return (N.X > 1) + (N.Y > 1) + (N.Z > 1);
}

} // namespace mg
