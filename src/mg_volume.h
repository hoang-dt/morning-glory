#pragma once

#include "mg_common.h"
#include "mg_data_types.h"
#include "mg_error.h"
#include "mg_macros.h"
#include "mg_memory.h"

namespace mg {

/* parent's type can be extent*, grid*, volume*, extent, grid, volume */
template <typename t = int*>
struct extent {
  u64 From = 0, Dims = 0;
  t Base = {};
  extent();
  extent(const v3i& Dims3);
  extent(const v3i& From3, const v3i& Dims3);
  template <typename u> extent(const extent<u>& Other);
  template <typename u> extent& operator=(const extent<u>& Other);
  bool HasBase() const;
};

/* parent's type can be extent*, grid*, volume*, extent, grid, volume */
template <typename t = int*>
struct grid {
  u64 From = 0, Dims = 0, Strd = 0; // packed from, dims, strides
  t Base = {};
  grid();
  grid(const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3);
  grid(const extent<t>& Ext);
  template <typename u> grid(const grid<u>& Other);
  template <typename u> grid& operator=(const grid<u>& Other);
  bool HasBase() const;
};

struct grid_indexer {
  //gird_indexer(v3i );
};

struct volume {
  buffer Buffer = {};
  u64 Dims = 0;
  data_type Type = data_type::__Invalid__;
  volume();
  volume(const buffer& Buf, const v3i& Dims3, data_type TypeIn);
  bool HasBase() const { return false; }
};

bool operator==(const volume& V1, const volume& V2);

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

mg_T(t) grid<volume> GridVolume(const t& Invalid);
mg_T(t) grid<volume> GridVolume(const extent<t>& Ext);
mg_T(t) grid<volume> GridVolume(const grid<t>& Grid);
        grid<volume> GridVolume(const volume& Volume);
/* assumption: Grid1 is on top of Grid2 */
grid<volume> GridCollapse(const grid<volume>& Grid1, const grid<volume>& Grid2);

/* Read a volume from a file */
error<> ReadVolume(cstr FileName, const v3i& Dims3, data_type Type, volume* Vol);

/* Copy a region of the first volume to a region of the second volume */
void Copy(grid<volume>* Dst, const grid<volume>& Src);

void Clone(volume* Dst, volume& Src, allocator* Alloc = &Mallocator());

/* Return the number of dimensions, given a volume size */
int NumDims(const v3i& N);

} // namespace mg

#include "mg_volume.inl"
