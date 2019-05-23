#include "mg_assert.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_volume.h"

namespace mg {

//grid_volume
//GridCollapse(const grid& Top, const grid_volume& Bot) {
//  v3i From1 = From(Top), Dims1 = Dims(Top), Strd1 = Strd(Top);
//  v3i From2 = From(Bot), Dims2 = Dims(Bot), Strd2 = Strd(Bot);
//  mg_Assert(From1 + (Dims1 - 1) * Strd1 < Dims2);
//  return grid_volume{grid(From2 + Strd2 * From1, Dims1, Strd1 * Strd2), Bot.Base};
//}
//
//grid GridCollapse(const grid& Top, const grid& Bot) {
//  v3i From1 = From(Top), Dims1 = Dims(Top), Strd1 = Strd(Top);
//  v3i From2 = From(Bot), Dims2 = Dims(Bot), Strd2 = Strd(Bot);
//  mg_Assert(From1 + (Dims1 - 1) * Strd1 < Dims2);
//  return grid(From2 + Strd2 * From1, Dims1, Strd1 * Strd2);
//}

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

