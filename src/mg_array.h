#pragma once

#include "mg_macros.h"
#include "mg_memory.h"
#include "mg_common.h"

namespace mg {

/* Only works for POD types. For other types, use std::vector.
 * NOTE: elements must be explicitly initialized (they are not initialized to
 * zero or anything)
 * NOTE: do not make copies of a dynamic_array using operator=, then work on
 * them as if they were independent from the original object (i.e., a
 * dynamic_array does not assume ownership of its memory buffer)
*/
mg_T(t)
struct array {
  buffer Buffer;
  i64 Size = 0;
  i64 Capacity = 0;
  allocator* Alloc = nullptr;
  array(allocator* Alloc = &Mallocator());
  t& operator[](i64 Idx);
};

mg_T(t) void Init(array<t>* Array, i64 Size);
mg_T(t) void Init(array<t>* Array, i64 Size, const t& Val);

mg_T(t) i64 Size(array<t>& Array);

mg_T(t) t& Front(array<t>& Array);
mg_T(t) t& Back(array<t>& Array);
mg_T(t) t* Begin(array<t>& Array);
mg_T(t) t* End(array<t>& Array);

mg_T(t) void Clone(array<t>* Dst, array<t>& Src);
mg_T(t) void Relocate(array<t>* Array, buffer Buf);

mg_T(t) void SetCapacity(array<t>* Array, i64 NewCapacity = 0);
mg_T(t) void Resize(array<t>* Array, i64 NewSize);
mg_T(t) void Reserve(array<t>* Array, i64 Capacity);
mg_T(t) void Clear(array<t>* Array);

mg_T(t) void PushBack(array<t>* Array, const t& Item);

mg_T(t) void Dealloc(array<t>* Array);

} // namespace mg

#include "mg_array.inl"
