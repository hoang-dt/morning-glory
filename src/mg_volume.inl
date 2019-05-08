#pragma once

#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_math.h"

namespace mg {

mg_Ti(t) extent<t>::
extent() = default;

mg_Ti(t) extent<t>::
extent(const v3i& Dims3)
  : From(Pack3i64(v3i::Zero))
  , Dims(Pack3i64(Dims3)) {}

mg_Ti(t) extent<t>::
extent(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3)) {}

mg_T(t) mg_Ti(u) extent<t>::
extent(const extent<u>& Other)
  : From(Other.From)
  , Dims(Other.Dims) 
{ if constexpr (is_same_type<t, u>::Value) Base = Other.Base; }

mg_Ti(t) bool extent<t>::
HasBase() const {
  if constexpr (is_same_type<t, int*>::Value)
    return false;
  if constexpr (is_pointer<t>::Value)
    return (void*)Base != nullptr;
  return true;
}

mg_T(t) mg_Ti(u) extent<t>& extent<t>::
operator=(const extent<u>& Other) {
  From = Other.From;
  Dims = Other.Dims;
  if constexpr (is_same_type<t, u>::Value) 
    Base = Other.Base;
  return *this;
}

mg_Ti(t) grid<t>::
grid() = default;

mg_Ti(t) grid<t>::
grid(const v3i& Dims3)
  : From(Pack3i64(v3i::Zero))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(v3i::One)) {}

mg_Ti(t) grid<t>::
grid(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(v3i::One)) {}

mg_Ti(t) grid<t>::
grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(Strd3)) {}

mg_T(t) grid<t>::
grid(const extent<t>& Ext)
  : From(Ext.From)
  , Dims(Ext.Dims)
  , Strd(Pack3i64(v3i::One)) {}

mg_Ti(t) bool grid<t>::
HasBase() const {
  if constexpr (is_same_type<t, int*>::Value)
    return false;
  if constexpr (is_pointer<t>::Value)
    return (void*)Base != nullptr;
  return true;
}

mg_T(t) mg_Ti(u) grid<t>& grid<t>::
operator=(const grid<u>& Other) {
  From = Other.From;
  Dims = Other.Dims;
  Strd = Other.Strd;
  if constexpr (is_same_type<t, u>::Value) 
    Base = Other.Base;
  return *this;
}

mg_Inline volume::
volume() = default;

mg_Inline volume::
volume(const buffer& Buf, const v3i& Dims3, data_type TypeIn)
  : Buffer(Buf)
  , Dims(Pack3i64(Dims3))
  , Type(TypeIn) {}

mg_Inline bool
operator==(const volume& V1, const volume& V2) {
  return V1.Buffer == V2.Buffer && V1.Dims == V2.Dims && V1.Type == V2.Type;
}

mg_Ti(t) v3i From(const extent<t>& Ext) { return Unpack3i64(Ext.From); }
mg_Ti(t) v3i Dims(const extent<t>& Ext) { return Unpack3i64(Ext.Dims); }
mg_Ti(t) v3i Strd(const extent<t>& Ext) { return v3i::One; }
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

mg_Ti(t) grid<volume>
GridVolume(const t& Invalid) { return grid<volume>(); }

mg_T(t) grid<volume>
GridVolume(const extent<t>& Ext) {
  grid<volume> MyGrid;
  MyGrid.From = Ext.From; MyGrid.Dims = Ext.Dims; MyGrid.Strd = Pack3i64(v3i::One);
  if (Ext.HasBase()) {
    grid<volume> BaseGrid = GridVolume(Value(Ext.Base));
    return GridCollapse(MyGrid, BaseGrid);
  }
  return MyGrid;
}

mg_T(t) grid<volume>
GridVolume(const grid<t>& Grid) {
  grid<volume> MyGrid;
  MyGrid.From = Grid.From; MyGrid.Dims = Grid.Dims; MyGrid.Strd = Grid.Strd;
  if (Grid.HasBase()) {
    grid<volume> BaseGrid = GridVolume(Value(Grid.Base));
    return GridCollapse(MyGrid, BaseGrid);
  }
  return MyGrid;
}

grid<volume>
GridVolume(const volume& Volume) {
  grid<volume> Result(Dims(Volume));
  Result.Base = Volume;
  return Result;
}

mg_Inline i64
Row(const v3i& N, const v3i& P) { return i64(P.Z) * N.X * N.Y + i64(P.Y) * N.X + P.X; }

mg_Inline v3i
InvRow(i64 I, const v3i& N) {
  i32 Z = i32(I / (N.X * N.Y));
  i32 XY = i32(I % (N.X * N.Y));
  return v3i(XY % N.X, XY / N.X, Z);
}

mg_Inline int
NumDims(const v3i& N) { return (N.X > 1) + (N.Y > 1) + (N.Z > 1); }

} // namespace mg
