#pragma once
//TODO: simplify zeroing arrays

#include "mg_types.h"

namespace mg {

template <typename t>
void AllocateTypedBuffer(typed_buffer<t>* Buf, i64 Size, allocator* Alloc) {
  buffer RawBuf;
  AllocateBuffer(&RawBuf, i64(Size * sizeof(t)), Alloc);
  Buf->Data = (t*)RawBuf.Data;
  Buf->Size = Size;
}
template <typename t>
void AllocateTypedBufferZero(typed_buffer<t>* Buf, i64 Size, allocator* Alloc) {
  buffer RawBuf;
  AllocateBufferZero(&RawBuf, i64(Size * sizeof(t)), Alloc);
  Buf->Data = (t*)RawBuf.Data;
  Buf->Size = Size;
}

template <typename t>
void DeallocateTypedBuffer(typed_buffer<t>* Buf, allocator* Alloc) {
  buffer RawBuf{ (byte*)Buf->Data, i64(Buf->Size * sizeof(t)) };
  DeallocateBuffer(&RawBuf, Alloc);
}

} // namespace mg

#undef mg_HeapArray
#define mg_HeapArray(Name, Type, Size)\
  using namespace mg;\
  typed_buffer<Type> Name;\
  AllocateTypedBuffer(&Name, (Size));\
  mg_CleanUp(__LINE__, DeallocateTypedBuffer(&Name))

#undef mg_HeapArrayZero
#define mg_HeapArrayZero(Name, Type, Size)\
  using namespace mg;\
  typed_buffer<Type> Name;\
  AllocateTypedBufferZero(&Name, (Size));\
  mg_CleanUp(__LINE__, DeallocateTypedBuffer(&Name))

#undef mg_StackArrayOfHeapArrays
#define mg_StackArrayOfHeapArrays(Name, Type, StackArraySize, HeapArraySize)\
  using namespace mg;\
  typed_buffer<Type> Name[StackArraySize] = {}; \
  for (int I = 0; I < (StackArraySize); ++I)\
    AllocateTypedBuffer(&Name[I], (HeapArraySize));\
  mg_CleanUp(__LINE__, { for (int I = 0; I < (StackArraySize); ++I) DeallocateTypedBuffer(&Name[I]); })
