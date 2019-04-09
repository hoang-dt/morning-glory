#pragma once

#include "mg_common_types.h"
#include "mg_error.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

struct extent {
  u64 PosCompact;
  u64 DimsCompact;
  extent();
  extent(v3i Dims);
  extent(v3i Pos, v3i Dims);
};

struct volume {
  buffer Buffer;
  extent Extent;
  u64 DimsCompact;
  data_type Type;
};

v3i Pos(extent Ext);
v3i Dims(extent Ext);
v3i BigDims(const volume& Vol);
v3i SmallDims(const volume& Vol);
i64 Size(const volume& Vol);

volume SubVolume(); // TODO: extract a subvolume from a volume

template <typename t> t& At(volume& Vol, i64 I);
template <typename t> t At(const volume& Vol, i64 I);

i64 XyzToI(v3i N, v3i P);
v3i IToXyz(i64 I, v3i N);

/* Read a volume from a file */
error<> ReadVolume(cstr FileName, v3i Dims, data_type Type, volume* Volume);

/* Copy a region of the first volume to a region of the second volume */
void Copy(volume* Dst, const volume& Src);
/* Clone a volume */
void Clone(volume* Dst, const volume& Src, allocator* Alloc = &Mallocator());

/* Split a volume into 8 parts: 1 voxel, 3 lines, 3 faces, and one sub volume */
array<extent, 8> Split3D(v3i Dims);

/* Return the number of dimensions, given a volume size */
int NumDims(v3i N);

bool IsPoint(extent Ext);
/* Note: must test for IsPoint() first */
bool IsLine(extent Ext);
/* Note: must test for IsLine() first */
bool IsFace(extent Ext);

} // namespace mg

#include "mg_volume.inl"
