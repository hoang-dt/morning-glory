#pragma once

#include "mg_algorithm.h"
#include "mg_assert.h"

namespace mg {

mg_Ti(t) array<t>::
array(allocator* Alloc) :
  Buffer(), Size(0), Capacity(0), Alloc(Alloc) { mg_Assert(Alloc); }

mg_Ti(t) array<t>::
array(const std::initializer_list<t>& List, allocator* Alloc) :
  array(Alloc) 
{
  mg_Assert(Alloc);
  Init(this, List.size());
  int Count = 0;
  for (const auto& Elem : List)
    (*this)[Count++] = Elem;
}

mg_Ti(t) t& array<t>::
operator[](i64 Idx) const {
  mg_Assert(Idx < Size);
  return const_cast<t&>(((t*)Buffer.Data)[Idx]);
}

mg_Ti(t) void
Init(array<t>* Array, i64 Size){
  Array->Alloc->Alloc(&Array->Buffer, Size * sizeof(t));
  Array->Size = Array->Capacity = Size;
}

mg_Ti(t) void
Init(array<t>* Array, i64 Size, const t& Val) {
  Init(Array, Size);
  Fill((t*)Array->Buffer.Data, (t*)Array->Buffer.Data + Size, Val);
}

mg_Ti(t) i64
Size(const array<t>& Array) { return Array.Size; }

mg_Ti(t) t&
Front(const array<t>& Array) {
  mg_Assert(Size(Array) > 0);
  return const_cast<t&>(Array[0]);
}

mg_Ti(t) t&
Back(const array<t>& Array) {
  mg_Assert(Size(Array) > 0);
  return const_cast<t&>(Array[Size(Array) - 1]);
}

mg_Ti(t) t*
Begin(const array<t>& Array) {
  return (t*)const_cast<byte*>(Array.Buffer.Data);
}
mg_Ti(t) t*
End(const array<t>& Array) {
  return (t*)const_cast<byte*>(Array.Buffer.Data) + Array.Size;
}

mg_T(t) void
Relocate(array<t>* Array, buffer Buf) {
  MemCopy(Array->Buffer, &Buf);
  Array->Alloc->Dealloc(&Array->Buffer);
  Array->Buffer = Buf;
  Array->Capacity = Buf.Bytes / sizeof(t);
  mg_Assert(Array->Size <= Array->Capacity);
}

mg_T(t) void
SetCapacity(array<t>* Array, i64 NewCapacity) {
  if (NewCapacity == 0) // default
    NewCapacity = Array->Capacity * 3 / 2 + 8;
  if (Array->Capacity < NewCapacity) {
    buffer Buf;
    Array->Alloc->Alloc(&Buf, NewCapacity * sizeof(t));
    mg_Assert(Buf);
    Relocate(Array, Buf);
    Array->Capacity = NewCapacity;
  }
}

mg_Ti(t) void
PushBack(array<t>* Array, const t& Item) {
  if (Array->Size >= Array->Capacity)
    SetCapacity(Array);
  (*Array)[Array->Size++] = Item;
}

mg_Ti(t) void
Clear(array<t>* Array) { Array->Size = 0; }

mg_T(t) void
Resize(array<t>* Array, i64 NewSize) {
  if (NewSize > Array->Capacity)
    SetCapacity(Array, NewSize);
  if (Array->Size < NewSize)
    Fill(Begin(*Array) + Array->Size, Begin(*Array) + NewSize, t{});
  Array->Size = NewSize;
}

mg_Ti(t) void
Reserve(array<t>* Array, i64 Capacity) {
  SetCapacity(Array, Capacity);
}

// TODO: test to see if t is POD, if yes, just memcpy
mg_T(t) void
Clone(const array<t>& Src, array<t>* Dst) {
  Resize(Dst, Size(Src));
  for (int I = 0; I < Size(Src); ++I)
    (*Dst)[I] = Src[I];
}

mg_T(t) void Print(printer* Pr, const array<t>& Array) {
  mg_Print(Pr, "[");
  for (i64 I = 0; I < Size(Array); ++I) 
    mg_Print(Pr, "%f, ", Array[I]); // TODO
  mg_Print(Pr, "]");
}

mg_Ti(t) void
Dealloc(array<t>* Array) {
  Array->Alloc->Dealloc(&Array->Buffer);
  Array->Size = Array->Capacity = 0;
}

} // namespace mg
