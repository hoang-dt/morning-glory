#pragma once

#include "mg_common_types.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

struct extent {
  u64 Pos;
  u64 Dims;
  extent();
  extent(v3i Dims);
  extent(v3i Pos, v3i Dims);
};

struct volume {
  buffer Buffer;
  u64 Dims;
  data_type Type;
};

struct sub_volume : public volume {
  extent Extent;
  sub_volume();
  sub_volume(volume Vol);
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

/* Split a volume into 8 parts: 1 voxel, 3 lines, 3 faces, and one sub volume */
array<extent, 8> Split3D(v3i Dims);

bool IsPoint(extent Ext);
/* Note: must test for IsPoint() first */
bool IsLine(extent Ext);
/* Note: must test for IsLine() first */
bool IsFace(extent Ext);

} // namespace mg

#include "mg_volume.inl"
