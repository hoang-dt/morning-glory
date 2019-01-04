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
  Volume->Block.BigDims = Volume->Block.SmallDims = Stuff3Ints(Dims);
  Volume->Block.Pos = Stuff3Ints(v3i(0, 0, 0));
  Volume->Type = Type;
  return mg_Error(NoError);
}

void Copy(volume* Dst, const volume& Src) {
#define Body(type)\
  mg_Assert(Src.Block.SmallDims == Dst->Block.SmallDims);\
  mg_Assert(Dst->Buffer.Data && Src.Buffer.Data);\
  mg_Assert(Dst->Type == Src.Type);\
  type* DstBuf = (type*)Dst->Buffer.Data;\
  const type* SrcBuf = (const type*)Src.Buffer.Data;\
  v3i StartSrc = Extract3Ints(Src.Block.Pos);\
  v3i StartDst = Extract3Ints(Dst->Block.Pos);\
  v3i Dims = Extract3Ints(Src.Block.SmallDims);\
  v3i BigDimsSrc = Extract3Ints(Src.Block.BigDims);\
  v3i BigDimsDst = Extract3Ints(Dst->Block.BigDims);\
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
  VolCopy.Block = Vol.Block;
  VolCopy.Buffer = Clone(Vol.Buffer, Alloc);
  VolCopy.Type = Vol.Type;
  return VolCopy;
}

} // namespace mg

