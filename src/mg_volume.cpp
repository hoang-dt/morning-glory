#include "mg_assert.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_volume.h"

namespace mg {

// TODO: add more return types to warn the user about the size of the volume loaded
// for example, the size of the volume may not agree with the input Dims3 and Type
// TODO: maybe add a mode where if the file size disagree with the inputs, then do
// not load the file
error<>
ReadVolume(cstr FileName, const v3i& Dims3, dtype Type, volume* Volume) {
  error Ok = ReadFile(FileName, &Volume->Buffer);
  if (Ok.Code != err_code::NoError)
    return Ok;
  *Volume = volume(Volume->Buffer, Dims3, Type);
  return mg_Error(err_code::NoError);
}

void
Resize(volume* Vol, const v3i& Dims3) {
  mg_Assert(Vol->Type != dtype::__Invalid__);
  i64 NewSize = Prod<u64>(Dims3) * SizeOf(Vol->Type);
  if (Size(Vol->Buffer) < NewSize)
    Resize(&Vol->Buffer, NewSize);
  SetDims(Vol, Dims3);
}

void 
Resize(volume* Vol, const v3i& Dims3, dtype Type) {
  auto OldType = Vol->Type;
  Vol->Type = Type;
  Resize(Vol, Dims3);
  if (Size(Vol->Buffer) < Prod<u64>(Dims3) * SizeOf(Vol->Type)) {
    Vol->Type = OldType;
  }
}

void
Dealloc(volume* Vol) {
  mg_Assert(Vol);
  DeallocBuf(&Vol->Buffer);
}

void
Clone(const volume& Src, volume* Dst, allocator* Alloc) {
  Clone(Src.Buffer, &Dst->Buffer, Alloc);
  Dst->Dims = Src.Dims;
  Dst->Type = Src.Type;
}

} // namespace mg

