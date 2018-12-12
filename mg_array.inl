#pragma once

namespace mg {

template <typename t>
dynamic_array<t>::dynamic_array(allocator* Alloc) : Buffer(), Size(0), Capacity(0), Alloc(Alloc) {
  mg_Assert(Alloc);
}

template <typename t>
const t& dynamic_array<t>::operator[](i64 Idx) const {
  mg_Assert(Idx < Size);
  return ((t*)Buffer.Data)[Idx];
}

template <typename t>
t& dynamic_array<t>::operator[](i64 Idx) {
  mg_Assert(Idx < Size);
  return ((t*)Buffer.Data)[Idx];
}

template <typename t>
void Init(dynamic_array<t>* Array, i64 Size){
  Alloc->Allocate(&Array->Buffer, Size * sizeof(t));
  Array->Size = Array->Capacity = Size;
}

template <typename t>
void Init(dynamic_array<t>* Array, i64 Size, const t& Val) {
  Init(Array, Size);
  Fill((t*)Buffer.Data, (t*)Buffer.Data + Size, Val);
}

template <typename t>
i64 Size(const dynamic_array<t>& Array) {
  return Array.Size;
}

template <typename t>
const t& Front(const dynamic_array<t>& Array) {
  mg_Assert(Size(Array) > 0);
  return Array[0];
}

template <typename t>
t& Front(dynamic_array<t>& Array) {
  mg_Assert(Size(Array) > 0);
  return Array[0];
}

template <typename t>
const t& Back(const dynamic_array<t>& Array) {
  mg_Assert(Size(Array) > 0);
  return Array[Size(Array) - 1];
}

template <typename t>
t& Back(dynamic_array<t>& Array) {
  mg_Assert(Size(Array) > 0);
  return Array[Size(Array) - 1];
}

template <typename t> t* Begin(dynamic_array<t>& Array) { return (t*)Array.Buffer.Data; }
template <typename t> t* End(dynamic_array<t>& Array) { return (t*)Array.Buffer.Data + Array.Size; }
template <typename t> const t* ConstBegin(const dynamic_array<t>& Array) { return (t*)Array.Buffer.Data; }
template <typename t> const t* ConstEnd(const dynamic_array<t>& Array) { return (t*)Array.Buffer.Data + Array.Size; }

template <typename t>
void MoveToNewBuffer(dynamic_array<t>* Array, buffer Buf) {
  MemCopy(&Buf, Array->Buffer);
  Array->Alloc->Deallocate(&Array->Buffer);
  Array->Buffer = Buf;
  Array->Capacity = Buf.Bytes / sizeof(t);
  mg_Assert(Array->Size <= Array->Capacity);
}

template <typename t>
void IncreaseCapacity(dynamic_array<t>* Array, i64 NewCapacity) {
  if (NewCapacity == 0) // default
    NewCapacity = Array->Capacity * 3 / 2 + 8;
  if (Array->Capacity < NewCapacity) {
    buffer Buf;
    Array->Alloc->Allocate(&Buf, NewCapacity * sizeof(t));
    MoveToNewBuffer(Array, Buf);
    Array->Capacity = NewCapacity;
  }
}

template <typename t>
void PushBack(dynamic_array<t>* Array, const t& Item) {
  if (Array->Size >= Array->Capacity)
    IncreaseCapacity(Array);
  (*Array)[Array->Size++] = Item;
}

template <typename t>
void Clear(dynamic_array<t>* Array) {
  Array->Size = 0;
}

template <typename t>
void Resize(dynamic_array<t>* Array, i64 NewSize) {
  if (NewSize > Array->Capacity)
    IncreaseCapacity(Array, NewSize);
  if (Array->Size < NewSize)
    Fill(Begin(*Array) + Array->Size, Begin(*Array) + NewSize, t{});
  Array->Size = NewSize;
}

template <typename t>
void Reserve(dynamic_array<t>* Array, i64 Capacity) {
  IncreaseCapacity(Array, Capacity);
}

template <typename t>
void DeallocateMemory(dynamic_array<t>* Array) {
  Alloc->Deallocate(&Array->Buffer);
  Array->Size = Array->Capacity = 0;
}

} // namespace mg
