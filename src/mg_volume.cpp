#include "mg_assert.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_volume.h"

namespace mg {

error<>
ReadVolume(cstr FileName, const v3i& Dims3, dtype Type, volume* Volume) {
  error Ok = ReadFile(FileName, &Volume->Buffer);
  if (Ok.Code != err_code::NoError)
    return Ok;
  *Volume = volume(Volume->Buffer, Dims3, Type);
  return mg_Error(err_code::NoError);
}

void
Clone(const volume& Src, volume* Dst, allocator* Alloc) {
  Clone(Src.Buffer, &Dst->Buffer, Alloc);
  Dst->Dims = Src.Dims;
  Dst->Type = Src.Type;
}

} // namespace mg

