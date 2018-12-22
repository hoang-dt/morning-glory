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

// TODO: forward this to the mallocator
void Allocate(byte** Ptr, i64 Bytes) {
  mg_Assert(!(*Ptr), "Pointer not freed before allocating new memory");
  *Ptr = (byte*)malloc(Bytes);
  mg_AbortIf(!(*Ptr), "Out of memory");
}

void Deallocate(byte** Ptr) {
  free(*Ptr);
  *Ptr = nullptr;
}

void AllocateBuffer(buffer* Buf, i64 Bytes) {
  Mallocator().Allocate(Buf, Bytes);
}

void DeallocateBuffer(buffer* Buf) {
  Mallocator().Deallocate(Buf);
}

bool mallocator::Allocate(buffer* Buf, i64 Bytes) {
  mg_Assert(!Buf->Data || Buf->Bytes == 0, "Buffer not freed before allocating new memory");
  mg::Allocate(&Buf->Data, Bytes);
  Buf->Bytes = Bytes;
  return true;
}

void mallocator::Deallocate(buffer* Buf) {
  free(Buf->Data);
  Buf->Data = nullptr;
  Buf->Bytes = 0;
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
    CurrentBytes += Bytes;
    return true;
  }
  return false;
}

void linear_allocator::Deallocate(buffer* Buf) {
  if (Buf->Data + Buf->Bytes == Block.Data + CurrentBytes) {
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
    Head = Head->Next;
    return true;
  }
  return Parent->Allocate(Buf, Bytes);
}

void free_list_allocator::Deallocate(buffer* Buf) {
  if (MinBytes <= Buf->Bytes && Buf->Bytes <= MaxBytes && Head) {
    node* P = (node*)(Buf->Data);
    P->Next = Head;
    Head = P;
  } else {
    return Parent->Deallocate(Buf);
  }
}

void free_list_allocator::DeallocateAll() { // Note: the client may want to call Parent->DeallocateAll() as well
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

buffer Clone(buffer Buf, allocator* Alloc) {
  buffer BufClone;
  Alloc->Allocate(&BufClone, Buf.Bytes);
  MemCopy(&BufClone, Buf);
  return BufClone;
}

} // namespace mg
