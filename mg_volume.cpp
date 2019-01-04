#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_common_types.h"
#include "mg_expected.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_types.h"
#include "mg_volume.h"

namespace mg {

error ReadVolume(cstr FileName, v3i Dims, data_type Type, volume* Volume) {
  error Ok = ReadFile(FileName, &Volume->Buffer);
  if (!Ok)
    return Ok;
  Volume->Dims = Stuff3Ints(Dims);
  Volume->Type = Type;
  return mg_Error(NoError);
}

void Copy(sub_volume* Dst, const sub_volume& Src) {
#define Body(type)\
  mg_Assert(Src.Extent.Dims == Dst->Extent.Dims);\
  mg_Assert(Dst->Buffer.Data && Src.Buffer.Data);\
  mg_Assert(Dst->Type == Src.Type);\
  type* DstBuf = (type*)Dst->Buffer.Data;\
  const type* SrcBuf = (const type*)Src.Buffer.Data;\
  v3i StartSrc = Extract3Ints(Src.Extent.Pos);\
  v3i StartDst = Extract3Ints(Dst->Extent.Pos);\
  v3i Dims = Extract3Ints(Src.Extent.Dims);\
  v3i BigDimsSrc = Extract3Ints(Src.Dims);\
  v3i BigDimsDst = Extract3Ints(Dst->Dims);\
  v3i PosSrc = StartSrc;\
  v3i PosDst = StartDst;\
  for (PosSrc.Z = StartSrc.Z, PosDst.Z = StartDst.Z;\
       PosSrc.Z < StartSrc.Z + Dims.Z; ++PosSrc.Z, ++PosDst.Z) {\
  for (PosSrc.Y = StartSrc.Y, PosDst.Y = StartDst.Y;\
       PosSrc.Y < StartSrc.Y + Dims.Y; ++PosSrc.Y, ++PosDst.Y) {\
  for (PosSrc.X = StartSrc.X, PosDst.X = StartDst.X;\
       PosSrc.X < StartSrc.X + Dims.X; ++PosSrc.X, ++PosDst.X) {\
    i64 I = XyzToI(BigDimsSrc, PosSrc);\
    i64 J = XyzToI(BigDimsDst, PosDst);\
    DstBuf[J] = SrcBuf[I];\
  }}}\

  TypeChooser(Src.Type);
#undef Body
}

volume Clone(const volume& Vol, allocator* Alloc) {
  volume VolCopy;
  VolCopy.Buffer = Clone(Vol.Buffer, Alloc);
  VolCopy.Type = Vol.Type;
  return VolCopy;
}

} // namespace mg

