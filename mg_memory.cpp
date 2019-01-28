#include <string.h>
#include "mg_assert.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

void MemCopy(buffer* Dst, const buffer& Src) {
  mg_Assert(Dst->Data, "Copy to null");
  mg_Assert(Src.Data || Src.Bytes == 0, "Copy from null");
  mg_Assert(Dst->Bytes >= Src.Bytes, "Copy to a smaller buffer");
  memcpy(Dst->Data, Src.Data, Src.Bytes);
}

void ZeroBuffer(buffer* Buf) {
  mg_Assert(Buf->Data);
  memset(Buf->Data, 0, Buf->Bytes);
}

void AllocateBuffer(buffer* Buf, i64 Bytes, allocator* Alloc) {
  Alloc->Allocate(Buf, Bytes);
}

void AllocateBufferZero(buffer* Buf, i64 Bytes, allocator* Alloc) {
  mg_Assert(!Buf->Data || Buf->Bytes == 0, "Buffer not freed before allocating new memory");
  if (Alloc == &Mallocator()) {
    Buf->Data = (byte*)calloc(Bytes, 1);
  }  else {
    AllocateBuffer(Buf, Bytes, Alloc);
    ZeroBuffer(Buf);
  }
  mg_AbortIf(!(Buf->Data), "Out of memory");
  Buf->Bytes = Bytes;
}

void DeallocateBuffer(buffer* Buf) {
  mg_Assert(Buf->Alloc);
  Buf->Alloc->Deallocate(Buf);
}

bool mallocator::Allocate(buffer* Buf, i64 Bytes) {
  mg_Assert(!Buf->Data || Buf->Bytes == 0, "Buffer not freed before allocating new memory");
  Buf->Data = (byte*)malloc(Bytes);
  mg_AbortIf(!(Buf->Data), "Out of memory");
  Buf->Bytes = Bytes;
  Buf->Alloc = this;
  return true;
}

void mallocator::Deallocate(buffer* Buf) {
  free(Buf->Data);
  Buf->Data = nullptr;
  Buf->Bytes = 0;
  Buf->Alloc = nullptr;
}

void mallocator::DeallocateAll() {
  // empty
}

linear_allocator::linear_allocator() = default;

linear_allocator::linear_allocator(buffer Buf) : Block(Buf) {}

bool linear_allocator::Allocate(buffer* Buf, i64 Bytes) {
  if (CurrentBytes + Bytes <= Block.Bytes) {
    Buf->Data = Block.Data + CurrentBytes;
    Buf->Bytes = Bytes;
    Buf->Alloc = this;
    CurrentBytes += Bytes;
    return true;
  }
  return false;
}

void linear_allocator::Deallocate(buffer* Buf) {
  if (Buf->Data + Buf->Bytes == Block.Data + CurrentBytes) {
    Buf->Data = nullptr;
    Buf->Bytes = 0;
    Buf->Alloc = nullptr;
    CurrentBytes -= Buf->Bytes;
  }
}

void linear_allocator::DeallocateAll() {
  CurrentBytes = 0;
}

bool linear_allocator::Own(buffer Buf) {
  return Block.Data <= Buf.Data && Buf.Data < Block.Data + CurrentBytes;
}

free_list_allocator::free_list_allocator() = default;

free_list_allocator::free_list_allocator(i64 MinBytes, i64 MaxBytes, allocator* Parent)
  : MinBytes(MinBytes)
  , MaxBytes(MaxBytes)
  , Parent(Parent) {}

bool free_list_allocator::Allocate(buffer* Buf, i64 Bytes) {
  if (MinBytes <= Bytes && Bytes <= MaxBytes && Head) {
    Buf->Data = (byte*)Head;
    Buf->Bytes = Bytes;
    Buf->Alloc = this;
    Head = Head->Next;
    return true;
  }
  return Parent->Allocate(Buf, Bytes);
}

void free_list_allocator::Deallocate(buffer* Buf) {
  if (MinBytes <= Buf->Bytes && Buf->Bytes <= MaxBytes && Head) {
    Buf->Data = nullptr;
    Buf->Bytes = 0;
    Buf->Alloc = nullptr;
    node* P = (node*)(Buf->Data);
    P->Next = Head;
    Head = P;
  } else {
    return Parent->Deallocate(Buf);
  }
}

// NOTE: the client may want to call Parent->DeallocateAll() as well
void free_list_allocator::DeallocateAll() {
  while (Head) {
    node* Next = Head->Next;
    buffer Buf{ (byte*)Head, MaxBytes };
    Parent->Deallocate(&Buf);
    Head = Next;
  }
}

fallback_allocator::fallback_allocator() = default;

fallback_allocator::fallback_allocator(owning_allocator* Primary, allocator* Secondary)
  : Primary(Primary)
  , Secondary(Secondary) {}

bool fallback_allocator::Allocate(buffer* Buf, i64 Size) {
  bool Success = Primary->Allocate(Buf, Size);
  return Success ? Success : Secondary->Allocate(Buf, Size);
}

void fallback_allocator::Deallocate(buffer* Buf) {
  if (Primary->Own(*Buf))
    return Primary->Deallocate(Buf);
  Secondary->Deallocate(Buf);
}

void fallback_allocator::DeallocateAll() {
  Primary->DeallocateAll();
  Secondary->DeallocateAll();
}

void Clone(buffer* Dst, buffer Src, allocator* Alloc) {
  if (Dst->Data && Dst->Bytes != Src.Bytes)
    DeallocateBuffer(Dst);
  if (!Dst->Data && Dst->Bytes == 0)
    Alloc->Allocate(Dst, Src.Bytes);
  MemCopy(Dst, Src);
}

} // namespace mg
