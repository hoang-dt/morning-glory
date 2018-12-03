#pragma once

#include <stdlib.h>
#include "mg_assert.h"
#include "mg_types.h"

namespace mg {

template <typename t>
bool Allocate(t** Ptr, i64 Size) {
  mg_Assert(!(*Ptr), "Pointer not freed before allocating new memory");
  return (*Ptr = (t*)malloc(Size * sizeof(t))) != nullptr;
}

template <typename t>
void Deallocate(t** Ptr) {
  free(*Ptr);
  *Ptr = nullptr;
}

struct mallocator : public allocator {
  bool Allocate(buffer* Buf, i64 Size) override;
  bool Deallocate(buffer* Buf) override;
  bool DeallocateAll() override;
};

struct linear_allocator : public owning_allocator {
  buffer Block;
  i64 CurrentSize = 0;
  linear_allocator() = default;
  linear_allocator(buffer Buf);
  bool Allocate(buffer* Buf, i64 Size) override;
  bool Deallocate(buffer* Buf) override;
  bool DeallocateAll() override;
  bool Own(buffer Buf) override;
};

template <int Capacity>
struct stack_linear_allocator : public linear_allocator {
  array<byte, Capacity> Storage;
  stack_linear_allocator() : linear_allocator(buffer{ Storage, Capacity }) {}
};

struct free_list_allocator : public allocator {
  struct node { node* Next = nullptr; };
  node* Head = nullptr;
  i64 MinSize = 0;
  i64 MaxSize = 0;
  allocator* Parent = nullptr;
  free_list_allocator() = default;  
  free_list_allocator(i64 MinSize, i64 MaxSize, allocator* Parent = nullptr);
  bool Allocate(buffer* Buf, i64 Size) override;
  bool Deallocate(buffer* Buf) override;
  bool DeallocateAll() override;
};

struct fallback_allocator : public allocator {
  owning_allocator* Primary = nullptr;
  allocator* Secondary = nullptr;
  fallback_allocator() = default;
  fallback_allocator(owning_allocator* Primary, allocator* Secondary);
  bool Allocate(buffer* Buf, i64 Size) override;
  bool Deallocate(buffer* Buf) override;
};

} // namespace mg
