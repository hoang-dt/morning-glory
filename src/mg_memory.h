#pragma once

// TODO: think about thread safety
// TODO: (double-ended) StackAllocator
// TODO: aligned allocation
// TODO: pool allocator
// TODO: add asserts

#include <stdlib.h>
#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

/* General purpose buffer for string-related operations */
inline thread_local char ScratchBuf[1024];

/*
Quickly declare a heap-allocated array (typed_buffer) which deallocates itself
at the end of scope. */
#define mg_MallocArray (Name, Type, Size)
#define mg_MallocArray0(Name, Type, Size) // initialize elements to 0
#define mg_ArrayOfMallocArrays(Name, Type, SizeOuter, SizeInner)
#define mg_MallocArrayOfArrays(Name, Type, SizeOuter, SizeInner)

struct allocator {
  virtual bool Alloc(buffer* Buf, i64 Bytes) = 0;
  virtual void Dealloc(buffer* Buf) = 0;
  virtual void DeallocAll() = 0;
  virtual ~allocator() {}
};

/* Allocators that know if they own an address/buffer */
struct owning_allocator : allocator {
  virtual bool Own(const buffer& Buf) const = 0;
};

struct mallocator;
/* A simple allocator that allocates simply by bumping a counter */
struct linear_allocator;
/* A linear allocator that uses stack storage. */
template <int Capacity>
struct stack_linear_allocator;
/*
Whenever an allocation of a size in a specific range is made, return the block
immediately from the head of a linked list. Otherwise forward the allocation to
some Parent allocator. */
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
  linear_allocator(const buffer& Buf);
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
  bool Own(const buffer& Buf) const override;
};

template <int Capacity>
struct stack_linear_allocator : public linear_allocator {
  stack_array<byte, Capacity> Storage;
  stack_linear_allocator() : linear_allocator(buffer{Storage.Arr, Capacity}) {}
};

struct free_list_allocator : public allocator {
  struct node { node* Next = nullptr; };
  node* Head = nullptr;
  i64 MinBytes = 0;
  i64 MaxBytes = 0;
  allocator* Parent = nullptr;
  free_list_allocator();
  free_list_allocator(i64 MinBytesIn, i64 MaxBytesIn, allocator* ParentIn = nullptr);
  free_list_allocator(i64 Bytes, allocator* ParentIn = nullptr);
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
};

struct fallback_allocator : public allocator {
  owning_allocator* Primary = nullptr;
  allocator* Secondary = nullptr;
  fallback_allocator();
  fallback_allocator(owning_allocator* PrimaryIn, allocator* SecondaryIn);
  bool Alloc(buffer* Buf, i64 Bytes) override;
  void Dealloc(buffer* Buf) override;
  void DeallocAll() override;
};

static inline mallocator& Mallocator() {
  static mallocator Instance;
  return Instance;
}

void Clone(const buffer& Src, buffer* Dst, allocator* Alloc = &Mallocator());

/* Abstract away memory allocations/deallocations */
void AllocBuf  (buffer* Buf, i64 Bytes, allocator* Alloc = &Mallocator());
void AllocBuf0 (buffer* Buf, i64 Bytes, allocator* Alloc = &Mallocator());
void DeallocBuf(buffer* Buf);

mg_T(t) void AllocTypedBuf  (buffer_t<t>* Buf, i64 Size, allocator* Alloc = &Mallocator());
mg_T(t) void AllocTypedBuf0 (buffer_t<t>* Buf, i64 Size, allocator* Alloc = &Mallocator());
mg_T(t) void DeallocTypedBuf(buffer_t<t>* Buf);

/* Zero out a buffer */
void ZeroBuf(buffer* Buf);
mg_T(t) void ZeroTypedBuf(buffer_t<t>* Buf);
/* Copy one buffer to another. Here the order of arguments are the reverse of memcpy. */
void MemCopy(const buffer& Src, buffer* Dst);

} // namespace mg

#include "mg_memory.inl"
