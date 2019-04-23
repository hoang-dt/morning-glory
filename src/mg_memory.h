#pragma once

// TODO: think about thread safety
// TODO: (double-ended) StackAllocator
// TODO: aligned allocation
// TODO: pool allocator
// TODO: add asserts

#include <stdlib.h>
#include "mg_assert.h"
#include "mg_types.h"

namespace mg {

/* General purpose buffer for string-related operations */
thread_local char ScratchBuf[1024];

/* Quickly declare a heap-allocated array which deallocates itself at the end of
 * scope return a typed_buffer */
#define mg_HeapArray(Name, Type, Size)
#define mg_HeapArray0(Name, Type, Size)
/* return an array of typed_buffer */
#define mg_StackHeapArrays(Name, Type, StackArraySize, HeapArraySize)
#define mg_HeapHeapArrays(Name, Type, SizeOuter, SizeInner)

struct allocator {
  virtual bool Alloc(buffer* Buf, i64 Bytes) = 0;
  virtual void Dealloc(buffer* Buf) = 0;
  virtual void DeallocAll() = 0;
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
/* Whenever an allocation of a size in a specific range is made, return the
 * block immediately from the head of a linked list. Otherwise forward the
 * allocation to some Parent allocator. */
struct free_list_allocator;
/* Try to allocate using one allocator first (the Primary), then if that fails,
 * use another allocator (the Secondary). */
struct fallback_allocator;

struct mallocator : public allocator {
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
};

struct linear_allocator : public owning_allocator {
  buffer Block;
  i64 CurrentBytes = 0;
  linear_allocator();
  linear_allocator(buffer Buf);
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
  bool Own(buffer Buf) override;
};

template <int Capacity>
struct stack_linear_allocator : public linear_allocator {
  array<byte, Capacity> Storage;
  stack_linear_allocator() : linear_allocator(buffer{ Storage.Arr, Capacity }) {}
};

struct free_list_allocator : public allocator {
  struct node { node* Next = nullptr; };
  node* Head = nullptr;
  i64 MinBytes = 0;
  i64 MaxBytes = 0;
  allocator* Parent = nullptr;
  free_list_allocator();
  free_list_allocator(i64 MinBytes, i64 MaxBytes, allocator* Parent = nullptr);
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
};

struct fallback_allocator : public allocator {
  owning_allocator* Primary = nullptr;
  allocator* Secondary = nullptr;
  fallback_allocator();
  fallback_allocator(owning_allocator* Primary, allocator* Secondary);
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
};

static mallocator& Mallocator() {
  static mallocator Instance;
  return Instance;
}

void Clone(buffer* Dst, buffer Src, allocator* Alloc = &Mallocator());

/* Abstract away memory allocations/deallocations */
void AllocBuf(buffer* Buf, i64 Bytes, allocator* Alloc = &Mallocator());
void AllocBuf0(buffer* Buf, i64 Bytes, allocator* Alloc = &Mallocator());
void DeallocBuf(buffer* Buf);
template <typename t>
void AllocBufT(typed_buffer<t>* Buf, i64 Size, allocator* Alloc = &Mallocator());
template <typename t>
void AllocBufT0(typed_buffer<t>* Buf, i64 Size, allocator* Alloc = &Mallocator());
template <typename t>
void DeallocBufT(typed_buffer<t>* Buf);

void ZeroBuf(buffer* Buf);
template <typename t> void ZeroBufT(typed_buffer<t>* Buf);
void MemCopy(buffer* Dst, const buffer& Src);


} // namespace mg

#include "mg_memory.inl"
