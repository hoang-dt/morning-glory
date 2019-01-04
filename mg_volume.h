#pragma once

#include "mg_common_types.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

struct block_bounds {
  u64 Pos;
  u64 SmallDims;
  u64 BigDims;
  block_bounds();
  block_bounds(v3i SmallDims, v3i BigDims);
  block_bounds(v3i Pos, v3i SmallDims, v3i BigDims);
};

struct volume {
  buffer Buffer;
  block_bounds Block;
  data_type Type;
};

i64 XyzToI(v3i N, v3i P);
v3i IToXyz(i64 I, v3i N);

struct data_type;
struct error;
/* Read a volume from a file */
error ReadVolume(cstr FileName, v3i Dims, data_type Type, volume* Volume);

/* Copy a region of the first volume to a region of the second volume */
void Copy(volume* Dst, const volume& Src);
/* Clone a volume */
volume Clone(const volume& Vol, allocator* Alloc = &Mallocator());

} // namespace mg

#include "mg_volume.inl"
