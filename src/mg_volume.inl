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

mg_Ti(t) extent<t>::
extent(const v3i& From3, const v3i& Dims3, const t& BaseIn)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Base(BaseIn)
{
  static_assert(!is_same_type<t, int*>::Value, "for int*, use the 2-argument version");
  mg_Assert(mg::Dims(Value(Base)) == v3i::Zero /* dummy base*/ ||
            (From3 + (Dims3 - 1) < mg::Dims(Value(Base))));
}

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
grid(const t& BaseIn)
  : From(Pack3i64(mg::From(Value(BaseIn))))
  , Dims(Pack3i64(mg::Dims(Value(BaseIn))))
  , Strd(Pack3i64(mg::Strd(Value(BaseIn))))
  , Base(BaseIn)
{
  static_assert(!is_same_type<t, int*>::Value, "template type cannot be int*");
  mg_Assert(mg::Dims(Value(Base)) == v3i::Zero /* dummy base*/ ||
            (From + Strd * (Dims - 1) < mg::Dims(Value(Base))));
}

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

mg_Ti(t) grid<t>::
grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3, const t& BaseIn)
  : From(Pack3i64(From3))
  , Dims(Pack3i64(Dims3))
  , Strd(Pack3i64(Strd3))
  , Base(BaseIn)
{
  static_assert(!is_same_type<t, int*>::Value, "for int*, use the 3-argument version");
  mg_Assert(mg::Dims(Value(Base)) == v3i::Zero /* dummy base*/ ||
            (From3 + Strd3 * (Dims3 - 1) < mg::Dims(Value(Base))));
}

mg_T(t) grid<t>::
grid(const extent<t>& Ext)
  : From(Ext.From)
  , Dims(Ext.Dims)
  , Strd(Pack3i64(v3i::One))
  , Base(Ext.Base) {}

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

mg_Ti(t) v3i From(const grid<t>& Grid) { return Unpack3i64(Grid.From); }
mg_Ti(t) v3i Dims(const grid<t>& Grid) { return Unpack3i64(Grid.Dims); }
mg_Ti(t) v3i Strd(const grid<t>& Grid) { return Unpack3i64(Grid.Strd); }

mg_Inline v3i From(const volume& Vol) { return v3i::Zero; }
mg_Inline v3i Dims(const volume& Vol) { return Unpack3i64(Vol.Dims); }
mg_Inline i64 Size(const volume& Vol) { return Prod<i64>(Dims(Vol)); }

mg_Inline grid<volume>
GridVolume(const volume& Volume) {
  grid<volume> Result(Dims(Volume));
  Result.Base = Volume;
  return Result;
}

mg_T2(t, u) grid<u>
GridCollapse(const grid<t>& Grid1, const grid<u>& Grid2) {
  v3i From1 = From(Grid1), Dims1 = Dims(Grid1), Strd1 = Strd(Grid1);
  v3i From2 = From(Grid2), Dims2 = Dims(Grid2), Strd2 = Strd(Grid2);
  mg_Assert(From1 + (Dims1 - 1) * Strd1 < Dims2);
  if constexpr (is_same_type<u, int*>::Value)
    return grid<u>(From2 + Strd2 * From1, Dims1, Strd1 * Strd2);
  else
    return grid<u>(From2 + Strd2 * From1, Dims1, Strd1 * Strd2, Grid2.Base);
}

mg_Ti(t) mg_Gi
Begin(grid<volume>& Grid) {
  grid_iterator<t> Iter;
  Iter.D = Dims(Grid); Iter.S = Strd(Grid); Iter.P = From(Grid);
  Iter.N = Dims(Grid.Base);
  Iter.Ptr = (t*)Grid.Base.Buffer.Data + Row(Iter.N, Iter.P);
  return Iter;
}

mg_Ti(t) mg_Gi
End(grid<volume>& Grid) {
  grid_iterator<t> Iter;
  v3i To3(0, 0, From(Grid).Z + Dims(Grid).Z * Strd(Grid).Z);
  Iter.Ptr = (t*)Grid.Base.Buffer.Data + Row(Dims(Grid.Base), To3);
  return Iter;
}

mg_Ti(t) mg_Gi& mg_Gi::
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

mg_Ti(t) t& mg_Gi::
operator*() { return *Ptr; }
mg_Ti(t) bool mg_Gi::
operator!=(const grid_iterator<t>& Other) const { return Ptr != Other.Ptr; }
mg_Ti(t) bool mg_Gi::
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
#define mg_BeginGridLoop2(G1, G2)\
  v3i Pos;\
  v3i FromDst = From(G1), FromSrc = From(G2);\
  v3i Dims3 = Dims(G1), DimsSrc = Dims((G1).Base), DimsDst = Dims((G2).Base);\
  v3i StrdSrc = Strd(G1), StrdDst = Strd(G2);\
  mg_BeginFor3(Pos, v3i::Zero, Dims3, v3i::One) {\
    i64 I = Row(DimsSrc, FromSrc + Pos * StrdSrc);\
    i64 J = Row(DimsDst, FromDst + Pos * StrdDst);\

#undef mg_EndGridLoop2
#define mg_EndGridLoop2 }}}

mg_T(t) void
Copy(grid<t>* Dst, const grid<t>& Src) {
  static_assert(is_same_type<t, volume>::Value || is_same_type<t, volume*>::Value);
#define Body(type)\
  mg_Assert(Dims(Src) == Dims(*Dst));\
  mg_Assert(Value(Dst->Base).Buffer && Value(Src.Base).Buffer);\
  mg_Assert(Value(Dst->Base).Type == Value(Src.Base).Type);\
  typed_buffer<type> DstBuf(Value(Dst->Base).Buffer), SrcBuf(Value(Src.Base).Buffer);\
  mg_BeginGridLoop2(Src, *Dst)\
    DstBuf[J] = SrcBuf[I];\
  }}}

  mg_DispatchOnType(Value(Src.Base).Type);
#undef Body
}

} // namespace mg
