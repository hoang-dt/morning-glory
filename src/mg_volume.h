#pragma once

#include "mg_common_types.h"
#include "mg_error.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

struct extent {
  u64 PosPacked;
  u64 DimsPacked;
  u64 StridesPacked;
  extent();
  extent(const v3i& Dims);
  extent(const v3i& Pos, const v3i& Dims);
  extent(const v3i& Pos, const v3i& Dims, const v3i& Stride);
};

struct volume {
  buffer Buffer;
  extent Extent;
  u64 DimsPacked;
  data_type Type;
  volume();
  volume(const buffer& BufIn, const extent& ExtIn, const v3i& DimsIn,
         data_type TypeIn);
};

v3i Pos(const extent& Ext);
v3i Dims(const extent& Ext);
v3i Strides(const extent& Ext);
v3i BigDims(const volume& Vol);
v3i SmallDims(const volume& Vol);
i64 Size(const volume& Vol);
void SetPos(extent* Ext, const v3i& Pos);
void SetDims(extent* Ext, const v3i& Dims);
void SetStrides(extent* Ext, const v3i& Strides);

volume SubVolume(); // TODO: extract a subvolume from a volume

//template <typename t> t& At(volume& Vol, i64 I);
//template <typename t> t At(const volume& Vol, i64 I);

i64 XyzToI(const v3i& N, const v3i& P);
v3i IToXyz(i64 I, const v3i& N);

/* Read a volume from a file */
error<> ReadVolume(cstr FileName, const v3i& Dims, data_type Type, volume* Volume);

/* Copy a region of the first volume to a region of the second volume */
void Copy(volume* Dst, const volume& Src);
/* Clone a volume */
void Clone(volume* Dst, const volume& Src, allocator* Alloc = &Mallocator());

/* Split a volume into 8 parts: 1 voxel, 3 lines, 3 faces, and one sub volume */
array<extent, 8> Split3D(const v3i& Dims);

/* Return the number of dimensions, given a volume size */
int NumDims(const v3i& N);

bool IsPoint(const extent& Ext);
/* Note: must test for IsPoint() first */
bool IsLine(const extent& Ext);
/* Note: must test for IsLine() first */
bool IsFace(const extent& Ext);

} // namespace mg

#include "mg_volume.inl"
