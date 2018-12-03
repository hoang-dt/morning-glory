#pragma once

// TODO: think about thread safety
// TODO: (double-ended) StackAllocator
// TODO: aligned allocation
// TODO: pool allocator

#include "mg_types.h"

namespace mg {

thread_local char ScratchBuffer[1024]; // General purpose buffer for string-related operations

/* Abstract away memory allocations/deallocations */
template <typename t>
bool Allocate(t** Ptr, i64 Size);

template <typename t>
void Deallocate(t** Ptr);

bool AllocateBuffer(buffer* Buf, i64 Size);
void DeallocateBuffer(buffer* Buf);

void MemCopy(buffer* Dst, const buffer& Src);

struct allocator {
  virtual bool Allocate(buffer* Buf, i64 Size) = 0;
  virtual bool Deallocate(buffer* Buf) = 0;
  virtual bool DeallocateAll() = 0;
};

/* Allocators that know if they own an address/buffer */
struct owning_allocator : allocator {
  virtual bool Own(buffer Buf) = 0;
};

struct mallocator;
/* A simple allocator that allocates simply by bumping a counter */
struct linear_allocator;
/* A linear allocator that uses stack storage. */
template <int Capacity>
struct stack_linear_allocator;
/* Whenever an allocation of a size in a specific range is made, return the block immediately
from the head of a linked list. Otherwise forward the allocation to some Parent allocator. */
struct free_list_allocator;
/* Try to allocate using one allocator first (the Primary), then if that fails, use another
allocator (the Secondary). */
struct fallback_allocator;

} // namespace mg

#include "mg_memory.inl"
