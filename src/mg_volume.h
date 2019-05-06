#pragma once

#include "mg_common.h"
#include "mg_data_types.h"
#include "mg_error.h"
#include "mg_macros.h"
#include "mg_memory.h"

namespace mg {

/* parent's type can be extent*, grid*, volume*, extent, grid, volume */
template <typename t = void*>
struct extent {
  u64 From, Dims;
  t Base = {};
  extent();
  extent(const v3i& Dims3);
  extent(const v3i& From3, const v3i& Dims3);
  bool HasBase() const { return (void*)Base == nullptr; }
};

/* parent's type can be extent*, grid*, volume*, extent, grid, volume */
template <typename t = void*>
struct grid {
  u64 From, Dims, Strd; // packed from, dims, strides
  t Base = {};
  grid();
  grid(const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3);
  template <typename u>
  grid(const extent<u>& Ext);
  bool HasBase() const { return (void*)Base == nullptr; }
};

struct grid_indexer {
  //gird_indexer(v3i );
};

struct volume {
  buffer Buffer;
  u64 Dims;
  data_type Type;
  volume();
  volume(const buffer& Buf, const v3i& Dims3, data_type TypeIn);
  bool HasBase() const { return false; }
};

mg_T(t) grid<volume> GridVol(const extent<t>& Extent);
mg_T(t) grid<volume> GridVol(const grid<t>& Grid);
mg_T(t) grid<volume> GridVol(const volume& Volume);

mg_T(t) v3i From(const extent<t>& Ext);
mg_T(t) v3i Dims(const extent<t>& Ext);
mg_T(t) v3i Strd(const extent<t>& Ext);

mg_T(t) v3i From(const grid<t>& Grid);
mg_T(t) v3i Dims(const grid<t>& Grid);
mg_T(t) v3i Strd(const grid<t>& Grid);
mg_T(t) void SetFrom(grid<t>* Grid, const v3i& From3);
mg_T(t) void SetDims(grid<t>* Grid, const v3i& Dims3);
mg_T(t) void SetStrd(grid<t>* Grid, const v3i& Strd3);

v3i  Dims(const volume& Vol);
void SetDims(volume* Vol, const v3i& Dims);
void SetType(volume* Vol, data_type Type);
i64  Size(const volume& Vol);

i64 Row(const v3i& N, const v3i& P);
v3i InvRow(i64 I, const v3i& N);

/* Read a volume from a file */
error<> ReadVolume(cstr FileName, const v3i& Dims3, data_type Type, volume* Vol);

/* Copy a region of the first volume to a region of the second volume */
void Copy(volume* Dst, volume& Src);

void Clone(volume* Dst, volume& Src, allocator* Alloc = &Mallocator());

/* Return the number of dimensions, given a volume size */
int NDims(const v3i& N);

} // namespace mg

#include "mg_volume.inl"
