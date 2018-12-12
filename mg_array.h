#pragma once

#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

/* Only works for POD types. For other types, use std::vector. */
// NOTE: elements must be explicitly initialized (they are not initialized to zero or anything)
// NOTE: do not make copies of a dynamic_array using operator=, then work on them as if they were
// independent from the original object (i.e., a dynamic_array does not assume ownership of its
// memory buffer)
template <typename t>
struct dynamic_array {
  buffer Buffer;
  i64 Size = 0;
  i64 Capacity = 0;
  allocator* Alloc = nullptr;
  dynamic_array(allocator* Alloc = &Mallocator());
  const t& operator[](i64 Idx) const;
  t& operator[](i64 Idx);
};

template <typename t>
void Init(dynamic_array<t>* Array, i64 Size);

template <typename t>
void Init(dynamic_array<t>* Array, i64 Size, const t& Val);

template <typename t>
i64 Size(const dynamic_array<t>& Array);

template <typename t>
const t& Front(const dynamic_array<t>& Array);

template <typename t>
t& Front(dynamic_array<t>& Array);

template <typename t>
const t& Back(const dynamic_array<t>& Array);

template <typename t>
t& Back(dynamic_array<t>& Array);

template <typename t> t* Begin(dynamic_array<t>& Array);
template <typename t> t* End(dynamic_array<t>& Array);
template <typename t> const t* ConstBegin(const dynamic_array<t>& Array);
template <typename t> const t* ConstEnd(const dynamic_array<t>& Array);

template <typename t>
void MoveToNewBuffer(dynamic_array<t>* Array, buffer Buf);

template <typename t>
void IncreaseCapacity(dynamic_array<t>* Array, i64 NewCapacity = 0);

template <typename t>
void PushBack(dynamic_array<t>* Array, const t& Item);

template <typename t>
void Clear(dynamic_array<t>* Array);

template <typename t>
void Resize(dynamic_array<t>* Array, i64 NewSize);

template <typename t>
void Reserve(dynamic_array<t>* Array, i64 Capacity);

template <typename t>
void DeallocateMemory(dynamic_array<t>* Array);

} // namespace mg

#include "mg_array.inl"
