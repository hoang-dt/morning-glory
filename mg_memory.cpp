#include <string.h>
#include "mg_assert.h"
#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

void MemCopy(buffer* Dst, const buffer& Src) {
  mg_Assert(Dst->Data, "Copy to null");
  mg_Assert(Src.Data, "Copy from null");
  mg_Assert(Dst->Size >= Src.Size, "Copy to a smaller buffer");
  memcpy(Dst->Data, Src.Data, Src.Size);
}

bool AllocateBuffer(buffer* Buf, i64 Size) {
  mg_Assert(!Buf->Data || Buf->Size == 0, "Buffer not freed before allocating new memory");
  if (Allocate(&Buf->Data, Size)) {
    Buf->Size = Size;
    return true;
  }
  return false;
}

void DeallocateBuffer(buffer* Buf) {
  Deallocate(&Buf->Data);
  Buf->Size = 0;
}

bool mallocator::Allocate(buffer* Buf, i64 Size) {
  mg_Assert(!Buf->Data || Buf->Size == 0, "Buffer not freed before allocating new memory");
  if (mg::Allocate(&Buf->Data, Size)) {
    Buf->Size = Size;
    return true;
  }
  return false;
}

bool mallocator::Deallocate(buffer* Buf) {
  free(Buf->Data);
  Buf->Data = nullptr;
  Buf->Size = 0;
  return true;
}

bool mallocator::DeallocateAll() {
  return false;
}


linear_allocator::linear_allocator(buffer Buf) : Block(Buf) {}

bool linear_allocator::Allocate(buffer* Buf, i64 Size) {
  if (CurrentSize + Size <= Block.Size) {
    Buf->Data = Block.Data + CurrentSize;
    Buf->Size = Size;
    CurrentSize += Size;
    return true;
  } else {
    return false;
  }
}

bool linear_allocator::Deallocate(buffer* Buf) { 
  (void)Buf;
  return false; 
}

bool linear_allocator::DeallocateAll() {
  CurrentSize = 0;
  return false;
}

bool linear_allocator::Own(buffer Buf) {
  return Block.Data <= Buf.Data && Buf.Data < Block.Data + CurrentSize;
}

free_list_allocator::free_list_allocator(i64 MinSize, i64 MaxSize, allocator* Parent)
  : MinSize(MinSize)
  , MaxSize(MaxSize)
  , Parent(Parent) {}

bool free_list_allocator::Allocate(buffer* Buf, i64 Size) {
  if (MinSize <= Size && Size <= MaxSize && Head) {
    Buf->Data = (byte*)Head;
    Buf->Size = Size;
    Head = Head->Next;
    return true;
  } else {
    return Parent->Allocate(Buf, Size);
  }
}

bool free_list_allocator::Deallocate(buffer* Buf) {
  if (MinSize <= Buf->Size && Buf->Size <= MaxSize && Head) {
    node* P = (node*)(Buf->Data);
    P->Next = Head;
    Head = P;
    return true;
  } else {
    return Parent->Deallocate(Buf);
  }
}

bool free_list_allocator::DeallocateAll() { // Note: the client may want to call Parent->DeallocateAll() as well
  while (Head) {
    node* Next = Head->Next;
    buffer Buf{ (byte*)Head, MaxSize };
    Parent->Deallocate(&Buf);
    Head = Next;
  }
  return true;
}

fallback_allocator::fallback_allocator(owning_allocator* Primary, allocator* Secondary)
  : Primary(Primary)
  , Secondary(Secondary) {}

bool fallback_allocator::Allocate(buffer* Buf, i64 Size) {
  bool Success = Primary->Allocate(Buf, Size);
  return Success ? Success : Secondary->Allocate(Buf, Size);
}

bool fallback_allocator::Deallocate(buffer* Buf) {
  if (Primary->Own(*Buf)) 
    return Primary->Deallocate(Buf);
  return Secondary->Deallocate(Buf);
}

} // namespace mg
