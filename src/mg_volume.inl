#pragma once

#include "mg_assert.h"
#include "mg_bitops.h"

namespace mg {

mg_Ti(t) extent<t>::
extent() = default;

mg_Ti(t) extent<t>::
extent(const v3i& Dims3)
  : From(Pack3i64(v3i::Zero()))
  , Dims(Pack3i64(Dims3)) {}

mg_Ti(t) extent<t>::
extent(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3)) {}

mg_Ti(t) grid<t>::
grid() = default;

mg_Ti(t) grid<t>::
grid(const v3i& Dims3)
  : From(Pack3i64(v3i::Zero()))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(v3i::One())) {}

mg_Ti(t) grid<t>::
grid(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(v3i::One())) {}

mg_Ti(t) grid<t>::
grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(Strd3)) {}

mg_T(t) mg_Ti(u) grid<t>::
grid(const extent<u>& Ext)
  : From(Ext.From)
  , Dims(Ext.Dims)
  , Strd(Pack3i64(v3i::One())) {}

mg_Inline volume::
volume() = default;

mg_Inline volume::
volume(const buffer& Buf, const v3i& Dims3, data_type TypeIn)
  : Buffer(Buf)
  , Dims(Pack3i64(Dims3))
  , Type(TypeIn) {}

mg_Ti(t) v3i From(const extent<t>& Ext) { return Unpack3i64(Ext.From); }
mg_Ti(t) v3i Dims(const extent<t>& Ext) { return Unpack3i64(Ext.Dims); }
mg_Ti(t) v3i Strd(const extent<t>& Ext) { return v3i::One(); }
mg_Ti(t) void SetFrom(extent<t>* Ext, const v3i& From3) { Ext->From = Pack3i64(From3); };
mg_Ti(t) void SetDims(extent<t>* Ext, const v3i& Dims3) { Ext->Dims = Pack3i64(Dims3); };

mg_Ti(t) v3i From(const grid<t>& Grid) { return Unpack3i64(Grid.From); }
mg_Ti(t) v3i Dims(const grid<t>& Grid) { return Unpack3i64(Grid.Dims); }
mg_Ti(t) v3i Strd(const grid<t>& Grid) { return Unpack3i64(Grid.Strd); }
mg_Ti(t) void SetFrom(grid<t>* Grid, const v3i& From3) { Grid->From = Pack3i64(From3); };
mg_Ti(t) void SetDims(grid<t>* Grid, const v3i& Dims3) { Grid->Dims = Pack3i64(Dims3); };
mg_Ti(t) void SetStrd(grid<t>* Grid, const v3i& Strd3) { Grid->Strd = Pack3i64(Strd3); };

mg_Inline v3i Dims(const volume& Vol) { return Unpack3i64(Vol.Dims); }
mg_Inline i64 Size(const volume& Vol) { return Prod<i64>(Dims(Vol)); }
mg_Inline void SetDims(volume* Vol, const v3i& Dims3) { Vol->Dims = Pack3i64(Dims3); }
mg_Inline void SetType(volume* Vol, data_type Type) { Vol->Type = Type; }

/* assumption: Grid1 is on top of Grid2 */
grid<volume>
GridVolume(grid<volume>& Grid1, grid<volume>& Grid2) {
  grid<volume> Result;
  v3i From1 = From(Grid1), Dims1 = Dims(Grid1), Strd1 = Strd(Grid1);
  v3i From2 = From(Grid2), Dims2 = Dims(Grid2), Strd2 = Strd(Grid2);
  mg_Assert(Dims1 <= Dims2);
  SetFrom(&Result, From2 + Strd2 * From1);
  SetStrd(&Result, Strd1 * Strd2);
  SetDims(&Result, Dims1);
  Result.Base = Grid2.Base;
  mg_Assert(From(Result) + Strd(Result) * (Dims(Result) - 1) <= Dims(Result.Base) - 1);
  return Result;
}

mg_T(t) grid<volume>
GridVolume(const extent<t>& Ext) {
  grid<volume> MyGrid(Ext);
  grid<volume> BaseGrid = Ext.HasBase() ? GridVolume(Value(Ext.Base)) : MyGrid;
  return GridVolume(MyGrid, BaseGrid);
}

mg_T(t) grid<volume>
GridVolume(const grid<t>& Grid) {
  grid<volume> BaseGrid = Grid.HasBase() ? GridVolume(Value(Grid.Base)) : Grid;
  return GridVolume(Grid, BaseGrid);
}

mg_T(t) grid<volume>
GridVolume(const volume& Volume) { return grid<volume>(Dims(Volume)); }

mg_Inline i64
Row(const v3i& N, const v3i& P) { return i64(P.Z) * N.X * N.Y + i64(P.Y) * N.X + P.X; }

mg_Inline v3i
InvRow(i64 I, const v3i& N) {
  i32 Z = i32(I / (N.X * N.Y));
  i32 XY = i32(I % (N.X * N.Y));
  return v3i(XY % N.X, XY / N.X, Z);
}

mg_Inline int
NDims(const v3i& N) { return (N.X > 1) + (N.Y > 1) + (N.Z > 1); }

} // namespace mg
