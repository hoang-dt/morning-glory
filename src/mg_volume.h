#pragma once

#include "mg_common.h"
#include "mg_data_types.h"
#include "mg_error.h"
#include "mg_macros.h"
#include "mg_memory.h"

namespace mg {

mg_T(t = int*) struct extent;
mg_T(t = int*) struct grid;
struct volume;

mg_T(t) struct is_extent                 : false_type {};
mg_T(t) struct is_extent<extent<t>>      : true_type  {};
mg_T(t) struct is_extent_ptr             : false_type {};
mg_T(t) struct is_extent_ptr<extent<t>*> : true_type  {};
mg_T(t) struct is_grid                   : false_type {};
mg_T(t) struct is_grid<grid<t>>          : true_type  {};
mg_T(t) struct is_grid_ptr               : false_type {};
mg_T(t) struct is_grid_ptr<grid<t>*>     : true_type  {};

/* parent's type can be extent*, grid*, volume*, extent, grid, volume */
template <typename t>
struct extent {
  using tt = typename remove_cv_ref<t>::type;
  static_assert(
    is_same_type<tt, int*  >::Value ||
    is_same_type<tt, volume>::Value || is_same_type <tt, volume*>::Value ||
    is_extent   <tt        >::Value || is_extent_ptr<tt         >::Value ||
    is_grid     <tt        >::Value || is_grid_ptr  <tt         >::Value,
    "Template base type must be one of int*, extent, grid, volume");

  u64 From = 0, Dims = 0;
  t Base = {};
  extent();
  explicit extent(const v3i& Dims3);
  extent(const v3i& From3, const v3i& Dims3);
  extent(const v3i& From3, const v3i& Dims, const t& BaseIn);
  template <typename u> extent(const extent<u>& Other);
  template <typename u> extent& operator=(const extent<u>& Other);
  bool HasBase() const;
};

/* parent's type can be extent*, grid*, volume*, extent, grid, volume */
template <typename t>
struct grid {
  using tt = typename remove_cv_ref<t>::type;
  static_assert(
    is_same_type<tt, int*  >::Value ||
    is_same_type<tt, volume>::Value || is_same_type <tt, volume*>::Value ||
    is_extent   <tt        >::Value || is_extent_ptr<tt         >::Value ||
    is_grid     <tt        >::Value || is_grid_ptr  <tt         >::Value,
    "Template base type must be one of int*, extent, grid, volume");

  u64 From = 0, Dims = 0, Strd = 0; // packed from, dims, strides
  t Base = {};
  grid();
  explicit grid(const v3i& Dims3);
  explicit grid(const t& BaseIn);
  grid(const v3i& From3, const v3i& Dims3);
  grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3);
  grid(const v3i& From3, const v3i& Dims3, const v3i& Strd3, const t& BaseIn);
  explicit grid(const extent<t>& Ext);
  template <typename u> grid(const grid<u>& Other);
  template <typename u> grid& operator=(const grid<u>& Other);
  bool HasBase() const;
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

v3i From(const volume& Vol);
v3i Dims(const volume& Vol);
v3i Strd(const volume& Vol);
i64 Size(const volume& Vol);

i64 Row(const v3i& N, const v3i& P);
v3i InvRow(i64 I, const v3i& N);

#define mg_Gi grid_iterator<t>

mg_T(t)
struct grid_iterator {
  t* Ptr = nullptr;
  v3i P = {}, D = {}, S = {}, N = {};
  grid_iterator& operator++();
  t& operator*();
  bool operator!=(const grid_iterator& Other) const;
  bool operator==(const grid_iterator& Other) const;
};

mg_T(t) mg_Gi Begin(grid<volume>& Grid);
mg_T(t) mg_Gi End(grid<volume>& Grid);

/* assumption: Grid1 is on top of Grid2 */
mg_T2(t, u) grid<u> GridCollapse(const grid<t>& Grid1, const grid<u>& Grid2);

/* Read a volume from a file */
error<> ReadVolume(cstr FileName, const v3i& Dims3, data_type Type, volume* Vol);

/* Copy a region of the first volume to a region of the second volume */
mg_T(t) void Copy(grid<t>* Dst, const grid<t>& Src);

void Clone(volume* Dst, volume& Src, allocator* Alloc = &Mallocator());

/* Return the number of dimensions, given a volume size */
int NumDims(const v3i& N);

#define mg_BeginGridLoop2(I, J) // loop through two grids in lockstep
#define mg_EndGridLoop2

} // namespace mg

#include "mg_volume.inl"
