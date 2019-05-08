#include "mg_assert.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_volume.h"

namespace mg {

error<>
ReadVolume(cstr FileName, const v3i& Dims3, data_type Type, volume* Volume) {
  error Ok = ReadFile(FileName, &Volume->Buffer);
  if (Ok.Code != err_code::NoError)
    return Ok;
  SetDims(Volume, Dims3);
  SetType(Volume, Type);
  return mg_Error(err_code::NoError);
}

void
Copy(grid<volume>* Dst, const grid<volume>& Src) {
#define Body(type)\
  mg_Assert(Dims(Src) == Dims(*Dst));\
  mg_Assert(Dst->Base.Buffer && Src.Base.Buffer);\
  mg_Assert(Dst->Base.Type == Src.Base.Type);\
  typed_buffer<type> DstBuf(Dst->Base.Buffer), SrcBuf(Src.Base.Buffer);\
  v3i From3Src = From(Src), From3Dst = From(*Dst);\
  v3i Dims3 = Dims(Src);\
  v3i Dims3Src = Dims(Src.Base), Dims3Dst = Dims(*Dst);\
  v3i Strd3Src = Strd(Src), Strd3Dst = Strd(*Dst);\
  v3i Pos;\
  mg_BeginFor3(Pos, v3i::Zero, Dims3, v3i::One) {\
    i64 I = Row(Dims3Src, From3Src + Pos * Strd3Src);\
    i64 J = Row(Dims3Dst, From3Dst + Pos * Strd3Dst);\
    DstBuf[J] = SrcBuf[I];\
  } mg_EndFor3\

  mg_DispatchOnType(Src.Base.Type);
#undef Body
}

grid<volume>
GridCollapse(const grid<volume>& Grid1, const grid<volume>& Grid2) {
  grid<volume> Result;
  v3i From1 = From(Grid1), Dims1 = Dims(Grid1), Strd1 = Strd(Grid1);
  v3i From2 = From(Grid2), Dims2 = Dims(Grid2), Strd2 = Strd(Grid2);
  mg_Assert(From1 + (Dims1 - 1) * Strd1 < Dims2);
  SetFrom(&Result, From2 + Strd2 * From1);
  SetStrd(&Result, Strd1 * Strd2);
  SetDims(&Result, Dims1);
  Result.Base = Grid2.Base;
  mg_Assert((Dims(Result.Base) == v3i::Zero) ||
            (From(Result) + Strd(Result) * (Dims(Result) - 1) < Dims(Result.Base)));
  return Result;
}

void
Clone(volume* Dst, const volume& Src, allocator* Alloc) {
  Clone(&Dst->Buffer, Src.Buffer, Alloc);
  Dst->Dims = Src.Dims;
  Dst->Type = Src.Type;
}

} // namespace mg

