#pragma once

#include "mg_common.h"
#include "mg_data_types.h"
#include "mg_error.h"
#include "mg_macros.h"
#include "mg_memory.h"

namespace mg {

struct volume;

struct extent {
  u64 From = 0, Dims = 0;
  extent();
  explicit extent(const v3i& Dims3);
  explicit extent(const volume& Vol);
  extent(const v3i& From3, const v3i& Dims3);
};

struct grid : public extent {
  u64 Strd = 0;
  grid();
  explicit grid(const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3);
  explicit grid(const extent& Ext);
};

struct volume {
  buffer Buffer = {};
  u64 Dims = 0;
  dtype Type = dtype::__Invalid__;
  volume();
  volume(const buffer& Buf, const v3i& Dims3, dtype TypeIn);
  volume(const v3i& Dims3, dtype TypeIn, allocator* Alloc = &Mallocator());
  mg_T(t) volume(t* Ptr, i64 Size);
  mg_T(t) volume(t* Ptr, const v3i& Dims3);
  mg_T(t) t& operator[](i64 I);
};

bool operator==(const volume& V1, const volume& V2);

v3i From(const extent& Ext);
v3i Dims(const extent& Ext);
v3i Strd(const extent& Ext);
i64 Size(const extent& Ext);

v3i From(const grid& Grid);
v3i Dims(const grid& Grid);
v3i Strd(const grid& Grid);
i64 Size(const grid& Grid);

v3i From(const volume& Vol);
v3i Dims(const volume& Vol);
v3i Strd(const volume& Vol);
i64 Size(const volume& Vol);

i64 Row(const v3i& N, const v3i& P);
v3i InvRow(i64 I, const v3i& N);

mg_T(t)
struct volume_iterator {
  t* Ptr = nullptr;
  v3i P = {}, N = {};
  volume_iterator& operator++();
  t& operator*();
  bool operator!=(const volume_iterator& Other) const;
  bool operator==(const volume_iterator& Other) const;
};
mg_T(t) volume_iterator<t> Begin(const volume& Vol);
mg_T(t) volume_iterator<t> End(const volume& Vol);

mg_T(t)
struct extent_iterator {
  t* Ptr = nullptr;
  v3i P = {}, D = {}, N = {};
  extent_iterator& operator++();
  t& operator*();
  bool operator!=(const extent_iterator& Other) const;
  bool operator==(const extent_iterator& Other) const;
};
mg_T(t) extent_iterator<t> Begin(const extent& Ext, const volume& Vol);
mg_T(t) extent_iterator<t> End(const extent& Ext, const volume& Vol);
// TODO: merge grid_iterator and grid_indexer?
// TODO: add extent_iterator and dimension_iterator?

mg_T(t)
struct grid_iterator {
  t* Ptr = nullptr;
  v3i P = {}, D = {}, S = {}, N = {};
  grid_iterator& operator++();
  t& operator*();
  bool operator!=(const grid_iterator& Other) const;
  bool operator==(const grid_iterator& Other) const;
};
mg_T(t) grid_iterator<t> Begin(const grid& Grid, const volume& Vol);
mg_T(t) grid_iterator<t> End(const grid& Grid, const volume& Vol);

///* assumption: Grid1 is on top of Grid2 */
//grid_volume GridCollapse(const grid& Top, const grid_volume& Bot);
//grid GridCollapse(const grid& Top, const grid& Bot);

/* Read a volume from a file */
error<> ReadVolume(cstr FileName, const v3i& Dims3, dtype Type, volume* Vol);

/* Copy a region of the first volume to a region of the second volume */
mg_T(t) void Copy(const t& SGrid, const volume& SVol, volume* DVol);
mg_T2(t1, t2) void Copy(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);

void Clone(const volume& Src, volume* Dst, allocator* Alloc = &Mallocator());

/* Return the number of dimensions, given a volume size */
int NumDims(const v3i& N);

#define mg_BeginGridLoop(G, V) // G is a grid and V is a volume
#define mg_EndGridLoop
#define mg_BeginGridLoop2(GI, VI, GJ, VJ) // loop through two grids in lockstep
#define mg_EndGridLoop2

} // namespace mg

#include "mg_volume.inl"
