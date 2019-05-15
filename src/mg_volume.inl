#pragma once

#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_math.h"

namespace mg {

mg_Inline extent::
extent() = default;

mg_Inline extent::
extent(const v3i& Dims3)
  : From(Pack3i64(v3i::Zero))
  , Dims(Pack3i64(Dims3)) {}

mg_Inline extent::
extent(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3)) {}

mg_Inline grid::
grid() = default;

mg_Inline grid::
grid(const v3i& Dims3)
  : From(Pack3i64(v3i::Zero))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline grid::
grid(const v3i& From3, const v3i& Dims3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline grid::
grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(Strd3)) {}

mg_Inline grid::
grid(const extent& Ext)
  : From(Ext.From)
  , Dims(Ext.Dims)
  , Strd(Pack3i64(v3i::One)) {}

mg_Inline volume::
volume() = default;

mg_Inline volume::
volume(const buffer& Buf, const v3i& Dims3, dtype TypeIn)
  : Buffer(Buf)
  , Dims(Pack3i64(Dims3))
  , Type(TypeIn) {}

mg_Ti(t) volume::
volume(t* Ptr, i64 Size)
  : Buffer((byte*)Ptr, Size * sizeof(t))
  , Dims(Pack3i64(v3i((i32)Size, 1, 1))) // NOTE the cast // TODO: maybe try to factor Size
  , Type(dtype_traits<t>::Type) { mg_Assert(Size <= (i64)traits<i32>::Max); }

mg_Ti(t) volume::
volume(t* Ptr, const v3i& Dims3)
  : Buffer((byte*)Ptr, Prod<i64>(Dims3) * sizeof(t))
  , Dims(Pack3i64(Dims3))
  , Type(dtype_traits<t>::Type) {}

mg_Ti(t) volume_indexer<t>::
volume_indexer(volume& Vol)
  : Buf(buffer<t>(Vol.Base))
  , BaseDims3(Dims(Vol.Base)) {}

mg_Ti(t) t& volume_indexer<t>::
At(const v3i& P) {
  i64 I = Row(BaseDims3, P);
  return Buf[I];
}

mg_Inline grid_volume::
grid_volume() = default;

mg_Inline grid_volume::
grid_volume(const volume& Vol)
  : Grid(Dims(Vol))
  , Base(Vol) {}

mg_Inline grid_volume::
grid_volume(const grid& GridIn, const volume& Vol)
  : Grid(GridIn)
  , Base(Vol) {}

mg_Inline grid_volume::
grid_volume(const v3i& Dims, const volume& Vol)
  : Grid(Dims)
  , Base(Vol) {}

mg_Inline grid_volume::
grid_volume(const extent& Ext, const volume& Vol)
  : Grid(Ext)
  , Base(Vol) {}

mg_Inline grid_volume::
grid_volume(const v3i& From3, const v3i& Dims3, const v3i& Strd3, const volume& Vol)
  : Grid(From3, Dims3, Strd3)
  , Base(Vol) {}

mg_Ti(t) grid_volume::
grid_volume(t* Ptr, i64 Size)
  : Grid(v3i(i32(Size), 1, 1))
  , Base(Ptr, Size) { mg_Assert(Size <= (i64)traits<i32>::Max); }

mg_Ti(t) grid_volume::
grid_volume(t* Ptr, const v3i& Dims3)
  : Grid(Dims3)
  , Base(Ptr, Dims3) {}

mg_Inline bool
operator==(const volume& V1, const volume& V2) {
  return V1.Buffer == V2.Buffer && V1.Dims == V2.Dims && V1.Type == V2.Type;
}

mg_Ti(t) grid_indexer<t>::
grid_indexer(grid_volume& Grid)
  : Buf(buffer<t>(Grid.Base.Buffer))
  , BaseDims3(Dims(Grid.Base))
  , GridFrom3(Dims(Grid)), GridStrd3(Strd(Grid)) {}

mg_Ti(t) t& grid_indexer<t>::
At(const v3i& P) {
  i64 I = Row(BaseDims3, GridFrom3 + P * GridStrd3);
  return Buf[I];
}

mg_Inline v3i From(const extent& Ext) { return Unpack3i64(Ext.From); }
mg_Inline v3i Dims(const extent& Ext) { return Unpack3i64(Ext.Dims); }
mg_Inline v3i Strd(const extent& Ext) { (void)Ext; return v3i::One; }
mg_Inline i64 Size(const extent& Ext) { return Prod<i64>(Dims(Ext)); }

mg_Inline v3i From(const grid& Grid) { return Unpack3i64(Grid.From); }
mg_Inline v3i Dims(const grid& Grid) { return Unpack3i64(Grid.Dims); }
mg_Inline v3i Strd(const grid& Grid) { return Unpack3i64(Grid.Strd); }
mg_Inline i64 Size(const grid& Grid) { return Prod<i64>(Dims(Grid)); };

mg_Inline v3i From(const volume& Vol) { (void)Vol; return v3i::Zero; }
mg_Inline v3i Dims(const volume& Vol) { return Unpack3i64(Vol.Dims); }
mg_Inline i64 Size(const volume& Vol) { return Prod<i64>(Dims(Vol)); }

mg_Inline v3i From(const grid_volume& Grid) { return From(Grid.Grid); }
mg_Inline v3i Dims(const grid_volume& Grid) { return Dims(Grid.Grid); }
mg_Inline v3i Strd(const grid_volume& Grid) { return Strd(Grid.Grid); }
mg_Inline i64 Size(const grid_volume& Grid) { return Size(Grid.Grid); }

mg_Ti(t) volume_iterator<t>
Begin(volume& Vol) {
  volume_iterator<t> Iter;
  Iter.P = v3i::Zero; Iter.N = Dims(Vol);
  Iter.Ptr = (t*)Vol.Buffer.Data;
  return Iter;
}

mg_Ti(t) volume_iterator<t>
End(volume& Vol) {
  volume_iterator<t> Iter;
  v3i To3(0, 0, Dims(Vol).Z);
  Iter.Ptr = (t*)Vol.Buffer.Data + Row(Dims(Vol), To3);
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

mg_Ti(t) grid_iterator<t>
Begin(grid_volume& Grid) {
  grid_iterator<t> Iter;
  Iter.D = Dims(Grid); Iter.S = Strd(Grid); Iter.P = From(Grid);
  Iter.N = Dims(Grid.Base);
  Iter.Ptr = (t*)Grid.Base.Buffer.Data + Row(Iter.N, Iter.P);
  return Iter;
}

mg_Ti(t) grid_iterator<t>
End(grid_volume& Grid) {
  grid_iterator<t> Iter;
  v3i To3(0, 0, From(Grid).Z + Dims(Grid).Z * Strd(Grid).Z);
  Iter.Ptr = (t*)Grid.Base.Buffer.Data + Row(Dims(Grid.Base), To3);
  return Iter;
}

mg_Ti(t) grid_iterator<t>& grid_iterator<t>::
operator++() {
  P.X += S.X;
  Ptr += S.X;
  if (P.X >= D.X) {
    P.X = 0;
    P.Y += S.Y;
    Ptr = Ptr - D.X * i64(S.X) + (N.X * S.Y);
    if (P.Y >= D.Y) {
      P.Y = 0;
      P.Z += S.Z;
      Ptr = Ptr - D.Y * i64(N.X) * S.Y + S.Z * i64(N.X) * N.Y;
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
#define mg_BeginGridLoop2(GI, GJ)\
  {\
    v3i Pos;\
    v3i FromI = From(GI), FromJ = From(GJ);\
    v3i Dims3 = Dims(GI), DimsI = Dims((GI).Base), DimsJ = Dims((GJ).Base);\
    v3i StrdI = Strd(GI), StrdJ = Strd(GJ);\
    mg_BeginFor3(Pos, v3i::Zero, Dims3, v3i::One) {\
      i64 I = Row(DimsI, FromI + Pos * StrdI);\
      i64 J = Row(DimsJ, FromJ + Pos * StrdJ);\

#undef mg_EndGridLoop2
#define mg_EndGridLoop2 }}}}

#undef mg_BeginGridLoop
#define mg_BeginGridLoop(G)\
  {\
    v3i Pos;\
    v3i From3 = From(G), Dims3 = Dims(G), Strd3 = Strd(G);\
    v3i DimsB = Dims((G).Base);\
    mg_BeginFor3(Pos, From3, Dims3, Strd3) {\
      i64 I = Row(DimsB, Pos);\

#undef mg_EndGridLoop
#define mg_EndGridLoop }}}}

} // namespace mg
