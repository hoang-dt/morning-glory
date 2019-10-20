#pragma once

#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_math.h"

namespace mg {

mg_Inline extent::
extent() = default;

mg_Inline extent::
extent(const v3i& Dims3)
  : From(0)
  , Dims(Pack3i64(Dims3)) {}

mg_Inline extent::
extent(const volume& Vol)
  : From(0)
  , Dims(Vol.Dims) {}

mg_Inline extent::
extent(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3)) {}

mg_Inline extent::
operator bool() const {
  return Dims != 0;
}

mg_Inline grid::
grid() = default;

mg_Inline grid::
grid(const v3i& Dims3)
  : extent(Dims3)
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline grid::
grid(const v3i& From3, const v3i& Dims3)
  : extent(From3, Dims3)
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline grid::
grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3)
  : extent(From3, Dims3)
  , Strd(Pack3i64(Strd3)) {}

mg_Inline grid::
grid(const extent& Ext)
  : extent(Ext)
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline grid::
grid(const volume& Vol)
  : extent(Vol)
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline grid::
operator bool() const {
  return Dims != 0;
}

mg_Inline volume::
volume() = default;

mg_Inline volume::
volume(const buffer& Buf, const v3i& Dims3, dtype TypeIn)
  : Buffer(Buf)
  , Dims(Pack3i64(Dims3))
  , Type(TypeIn) {}

mg_Inline volume::
volume(const v3i& Dims3, dtype TypeIn, allocator* Alloc)
  : Buffer()
  , Dims(Pack3i64(Dims3))
  , Type(TypeIn) { AllocBuf(&Buffer, SizeOf(TypeIn) * Prod<i64>(Dims3), Alloc); }

mg_Ti(t) volume::
volume(const t* Ptr, i64 Size)
  : volume(Ptr, v3i(Size, 1, 1)) { mg_Assert(Size <= (i64)traits<i32>::Max); }

mg_Ti(t) volume::
volume(const t* Ptr, const v3i& Dims3)
  : Buffer((byte*)const_cast<t*>(Ptr), Prod<i64>(Dims3) * sizeof(t))
  , Dims(Pack3i64(Dims3))
  , Type(dtype_traits<t>::Type) {}

mg_Ti(t) volume::
volume(const buffer_t<t>& Buf)
  : volume(Buf.Data, Buf.Size) {}

mg_Inline subvol_grid::
subvol_grid(const volume& VolIn)
  : Vol(VolIn)
  , Grid(VolIn) {}

mg_Inline subvol_grid::
subvol_grid(const grid& GridIn, const volume& VolIn)
  : Vol(VolIn)
  , Grid(GridIn) {}

mg_Ti(t) t& volume::
At(const v3i& P) const {
  return (const_cast<t*>((const t*)Buffer.Data))[Row(Unpack3i64(Dims), P)];
}

mg_Ti(t) t& volume::
At(i64 Idx) const {
  return (const_cast<t*>((const t*)Buffer.Data))[Idx];
}

mg_Inline bool
operator==(const volume& V1, const volume& V2) {
  return V1.Buffer == V2.Buffer && V1.Dims == V2.Dims && V1.Type == V2.Type;
}

mg_Inline v3i Dims(const v3i& Frst, const v3i& Last) { return Last - Frst + 1; }
mg_Inline v3i Dims(const v3i& Frst, const v3i& Last, const v3i& Strd) { return (Last - Frst) / Strd + 1; }

mg_Inline v3i From(const extent& Ext) { return Unpack3i64(Ext.From); }
mg_Inline v3i To(const extent& Ext) { return From(Ext) + Dims(Ext); }
mg_Inline v3i Frst(const extent& Ext) { return From(Ext); }
mg_Inline v3i Last(const extent& Ext) { return To(Ext) - 1; }
mg_Inline v3i Dims(const extent& Ext) { return Unpack3i64(Ext.Dims); }
mg_Inline v3i Strd(const extent& Ext) { (void)Ext; return v3i::One; }
mg_Inline i64 Size(const extent& Ext) { return Prod<i64>(Dims(Ext)); }
mg_Inline void SetFrom(extent& Ext, const v3i& From3) { Ext.From = Pack3i64(From3); }
mg_Inline void SetDims(extent& Ext, const v3i& Dims3) { Ext.Dims = Pack3i64(Dims3); }

mg_Inline v3i From(const grid& Grid) { return Unpack3i64(Grid.From); }
mg_Inline v3i To(const grid& Grid) { return From(Grid) + Dims(Grid) * Strd(Grid); }
mg_Inline v3i Frst(const grid& Grid) { return From(Grid); }
mg_Inline v3i Last(const grid& Grid) { return To(Grid) - Strd(Grid); }
mg_Inline v3i Dims(const grid& Grid) { return Unpack3i64(Grid.Dims); }
mg_Inline v3i Strd(const grid& Grid) { return Unpack3i64(Grid.Strd); }
mg_Inline i64 Size(const grid& Grid) { return Prod<i64>(Dims(Grid)); };
mg_Inline void SetFrom(grid& Grid, const v3i& From3) { Grid.From = Pack3i64(From3); }
mg_Inline void SetDims(grid& Grid, const v3i& Dims3) { Grid.Dims = Pack3i64(Dims3); }
mg_Inline void SetStrd(grid& Grid, const v3i& Strd3) { Grid.Dims = Pack3i64(Strd3); }

mg_Inline v3i From(const volume& Vol) { (void)Vol; return v3i::Zero; }
mg_Inline v3i To(const volume& Vol) { return Dims(Vol); }
mg_Inline v3i Frst(const volume& Vol) { return From(Vol); }
mg_Inline v3i Last(const volume& Vol) { return Dims(Vol) - 1; }
mg_Inline v3i Dims(const volume& Vol) { return Unpack3i64(Vol.Dims); }
mg_Inline i64 Size(const volume& Vol) { return Prod<i64>(Dims(Vol)); }

mg_Ti(t) volume_iterator<t>
Begin(const volume& Vol) {
  volume_iterator<t> Iter;
  Iter.P = v3i::Zero; Iter.N = Dims(Vol);
  Iter.Ptr = (t*)const_cast<byte*>(Vol.Buffer.Data);
  return Iter;
}

mg_Ti(t) volume_iterator<t>
End(const volume& Vol) {
  volume_iterator<t> Iter;
  v3i To3(0, 0, Dims(Vol).Z);
  Iter.Ptr = (t*)const_cast<byte*>(Vol.Buffer.Data) + Row(Dims(Vol), To3);
  return Iter;
}

mg_Ti(t) volume_iterator<t>& volume_iterator<t>::
operator++() {
  ++Ptr;
  if (++P.X >= N.X) {
    P.X = 0;
    if (++P.Y >= N.Y) {
      P.Y = 0;
      ++P.Z;
    }
  }
  return *this;
}

mg_Ti(t) t& volume_iterator<t>::
operator*() { return *Ptr; }

mg_Ti(t) bool volume_iterator<t>::
operator!=(const volume_iterator<t>& Other) const { return Ptr != Other.Ptr; }

mg_Ti(t) bool volume_iterator<t>::
operator==(const volume_iterator<t>& Other) const { return Ptr == Other.Ptr; }

mg_Ti(t) extent_iterator<t>
Begin(const extent& Ext, const volume& Vol) {
  extent_iterator<t> Iter;
  Iter.D = Dims(Ext); Iter.P = v3i(0); Iter.N = Dims(Vol);
  Iter.Ptr = (t*)const_cast<byte*>(Vol.Buffer.Data) + Row(Iter.N, From(Ext));
  return Iter;
}

mg_Ti(t) extent_iterator<t>
End(const extent& Ext, const volume& Vol) {
  extent_iterator<t> Iter;
  v3i To3 = From(Ext) + v3i(0, 0, Dims(Ext).Z);
  Iter.Ptr = (t*)const_cast<byte*>(Vol.Buffer.Data) + Row(Dims(Vol), To3);
  return Iter;
}

mg_Ti(t) extent_iterator<t>& extent_iterator<t>::
operator++() {
  ++P.X;
  ++Ptr;
  if (P.X >= D.X) {
    P.X = 0;
    ++P.Y;
    Ptr = Ptr - D.X + N.X;
    if (P.Y >= D.Y) {
      P.Y = 0;
      ++P.Z;
      Ptr = Ptr - D.Y * N.X + N.X * N.Y;
    }
  }
  return *this;
}

mg_Ti(t) t& extent_iterator<t>::
operator*() { return *Ptr; }

mg_Ti(t) bool extent_iterator<t>::
operator!=(const extent_iterator<t>& Other) const { return Ptr != Other.Ptr; }

mg_Ti(t) bool extent_iterator<t>::
operator==(const extent_iterator<t>& Other) const { return Ptr == Other.Ptr; }

mg_T(t) grid_iterator<t>
Begin(const grid& Grid, const volume& Vol) {
  grid_iterator<t> Iter;
  Iter.S = Strd(Grid); Iter.D = Dims(Grid) * Iter.S; Iter.P = v3i(0); Iter.N = Dims(Vol);
  Iter.Ptr = (t*)const_cast<byte*>(Vol.Buffer.Data) + Row(Iter.N, From(Grid));
  return Iter;
}

mg_T(t) grid_iterator<t>
End(const grid& Grid, const volume& Vol) {
  grid_iterator<t> Iter;
  v3i To3 = From(Grid) + v3i(0, 0, Dims(Grid).Z * Strd(Grid).Z);
  Iter.Ptr = (t*)const_cast<byte*>(Vol.Buffer.Data) + Row(Dims(Vol), To3);
  return Iter;
}

mg_T(t) grid_iterator<t>& grid_iterator<t>::
operator++() {
  P.X += S.X;
  Ptr += S.X;
  if (P.X >= D.X) {
    P.X = 0;
    P.Y += S.Y;
    Ptr = Ptr - D.X + (N.X * S.Y);
    if (P.Y >= D.Y) {
      P.Y = 0;
      P.Z += S.Z;
      Ptr = Ptr - D.Y * i64(N.X) + S.Z * i64(N.X) * N.Y;
    }
  }
  return *this;
}

mg_Ti(t) t& grid_iterator<t>::
operator*() { return *Ptr; }

mg_Ti(t) bool grid_iterator<t>::
operator!=(const grid_iterator<t>& Other) const { return Ptr != Other.Ptr; }

mg_Ti(t) bool grid_iterator<t>::
operator==(const grid_iterator<t>& Other) const { return Ptr == Other.Ptr; }

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

#undef mg_BeginGridLoop2
#define mg_BeginGridLoop2(GI, VI, GJ, VJ) /* GridI, VolumeI, GridJ, VolumeJ */\
  {\
    mg_Assert(Dims(GI) == Dims(GJ));\
    v3i Pos;\
    v3i FromI = From(GI), FromJ = From(GJ);\
    v3i Dims3 = Dims(GI), DimsI = Dims(VI), DimsJ = Dims(VJ);\
    v3i StrdI = Strd(GI), StrdJ = Strd(GJ);\
    mg_BeginFor3(Pos, v3i::Zero, Dims3, v3i::One) {\
      i64 I = Row(DimsI, FromI + Pos * StrdI);\
      i64 J = Row(DimsJ, FromJ + Pos * StrdJ);\

#undef mg_EndGridLoop2
#define mg_EndGridLoop2 }}}}

#undef mg_BeginGridLoop
#define mg_BeginGridLoop(G, V)\
  {\
    v3i Pos;\
    v3i From3 = From(G), Dims3 = Dims(G), Strd3 = Strd(G);\
    v3i DimsB = Dims(V);\
    mg_BeginFor3(Pos, From3, Dims3, Strd3) {\
      i64 I = Row(DimsB, Pos);\

#undef mg_EndGridLoop
#define mg_EndGridLoop }}}}

mg_T(t) void
Copy(const t& SGrid, const volume& SVol, volume* DVol) {
#define Body(type)\
  mg_Assert(Dims(SGrid) <= Dims(*DVol));\
  mg_Assert(Dims(SGrid) <= Dims(SVol));\
  mg_Assert(DVol->Buffer && SVol.Buffer);\
  mg_Assert(SVol.Type == DVol->Type);\
  auto SIt = Begin<type>(SGrid, SVol), SEnd = End<type>(SGrid, SVol);\
  auto DIt = Begin<type>(SGrid, *DVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = *SIt;

  mg_DispatchOnType(SVol.Type);
#undef Body
}

mg_TT(t1, t2) void
Copy(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
#define Body(type)\
  mg_Assert(Dims(SGrid) == Dims(DGrid));\
  mg_Assert(Dims(SGrid) <= Dims(SVol));\
  mg_Assert(Dims(DGrid) <= Dims(*DVol));\
  mg_Assert(DVol->Buffer && SVol.Buffer);\
  mg_Assert(SVol.Type == DVol->Type);\
  auto SIt = Begin<type>(SGrid, SVol), SEnd = End<type>(SGrid, SVol);\
  auto DIt = Begin<type>(DGrid, *DVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = *SIt;

  mg_DispatchOnType(SVol.Type);
#undef Body
}

mg_T(t) void 
Copy(const t& Grid, const subvol_grid& SVol, subvol_grid* DVol) {
  mg_Assert(Strd(SVol.Grid) % Strd(Grid) == 0);
  mg_Assert(Strd(DVol->Grid) % Strd(Grid) == 0);
  t Crop1 = Crop(Grid, SVol.Grid);
  if (Crop1) {
    t Crop2 = Crop(Crop1, DVol->Grid);
    if (Crop2) {
      grid SGrid = Relative(Crop2, SVol.Grid);
      grid DGrid = Relative(Crop2, DVol->Grid);
      Copy(SGrid, SVol.Volume, DGrid, &DVol->Volume);
    }
  }
}

mg_TT(t1, t2) void
Add(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
#define Body(type)\
  mg_Assert(Dims(SGrid) == Dims(DGrid));\
  mg_Assert(Dims(SGrid) <= Dims(SVol));\
  mg_Assert(Dims(DGrid) <= Dims(*DVol));\
  mg_Assert(DVol->Buffer && SVol.Buffer);\
  mg_Assert(SVol.Type == DVol->Type);\
  auto SIt = Begin<type>(SGrid, SVol), SEnd = End<type>(SGrid, SVol);\
  auto DIt = Begin<type>(DGrid, *DVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt += *SIt;

  mg_DispatchOnType(SVol.Type);
#undef Body
}

// TODO: what is this
mg_TT(t1, t2) void
Add2(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
  mg_Assert(Dims(SGrid) == Dims(DGrid));\
  mg_Assert(Dims(SGrid) <= Dims(SVol));\
  mg_Assert(Dims(DGrid) <= Dims(*DVol));\
  mg_Assert(DVol->Buffer && SVol.Buffer);\
  mg_Assert(SVol.Type == DVol->Type);\
  auto SBeg = Begin<f64>(SGrid, SVol);
  auto SIt = Begin<f64>(SGrid, SVol), SEnd = End<f64>(SGrid, SVol);\
  auto DIt = Begin<f64>(DGrid, *DVol), DEnd = End<f64>(DGrid, *DVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt += *SIt;
}

mg_TT(t1, t2) bool
IsSubGrid(const t1& Grid1, const t2& Grid2) {
  if (!(From(Grid1) >= From(Grid2)))
    return false;
  if (!(To(Grid1) <= To(Grid2)))
    return false;
  if ((Strd(Grid1) % Strd(Grid2)) != 0)
    return false;
  if ((From(Grid1) - From(Grid2)) % Strd(Grid2) != 0)
    return false;
  return true;
}

mg_TT(t1, t2) t1 
SubGrid(const t1& Grid1, const t2& Grid2) {
  return t1(From(Grid1) + Strd(Grid1) * From(Grid2), Dims(Grid2), Strd(Grid1) * Strd(Grid2));
}

mg_TT(t1, t2) t1
Relative(const t1& Grid1, const t2& Grid2) {
  mg_Assert(IsSubGrid(Grid1, Grid2));
  v3i From3 = (From(Grid1) - From(Grid2)) / Strd(Grid2);
  return grid(From3, Dims(Grid1), Strd(Grid1) / Strd(Grid2));
}

mg_TT(t1, t2) t1
Crop(const t1& Grid1, const t2& Grid2) {
  v3i Strd3 = Strd(Grid1);
  v3i Frst3 = ((Frst(Grid2) - 1) / Strd3 + 1) * Strd3;
  v3i Last3 = (Last(Grid2) / Strd3) * Strd3;
  t1 OutGrid = Grid1;
  Frst3 = Max(Frst3, Frst(Grid1));
  Last3 = Min(Last3, Last(Grid1));
  v3i Dims3 = Frst3 <= Last3 ? (Last3 - Frst3) / Strd3 + 1 : v3i::Zero;
  OutGrid.From = Pack3i64(Frst3);
  OutGrid.Dims = Pack3i64(Dims3);
  return OutGrid;
}

// TODO: this can be turned into a slice function ala Python[start:stop]
mg_T(t) t
Slab(const t& Grid, dimension D, int N) {
  v3i Dims3 = Dims(Grid);
  mg_Assert(abs(N) <= Dims3[D] && N != 0);
  t Slab = Grid;
  if (N < 0) {
    v3i From3 = From(Grid);
    v3i Strd3 = Strd(Grid);
    From3[D] += (Dims3[D] + N) * Strd3[D];
    SetFrom(Slab, From3);
  }
  Dims3[D] = abs(N);
  SetDims(Slab, Dims3);
  return Slab;
}

mg_T(t) t
Translate(const t& Grid, dimension D, int N) {
  v3i From3 = From(Grid);
  From3[D] += N;
  t Slab = Grid;
  SetFrom(Slab, From3);
  return Slab;
}

} // namespace mg
