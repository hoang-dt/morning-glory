#pragma once
//TODO: simplify zeroing arrays

#include "mg_types.h"

namespace mg {

template <typename t>
void AllocBufT(typed_buffer<t>* Buf, i64 Size, allocator* Alloc) {
  buffer RawBuf;
  AllocBuf(&RawBuf, i64(Size * sizeof(t)), Alloc);
  Buf->Data = (t*)RawBuf.Data;
  Buf->Size = Size;
  Buf->Alloc = Alloc;
}
template <typename t>
void AllocBufT0(typed_buffer<t>* Buf, i64 Size, allocator* Alloc) {
  buffer RawBuf;
  AllocBuf0(&RawBuf, i64(Size * sizeof(t)), Alloc);
  Buf->Data = (t*)RawBuf.Data;
  Buf->Size = Size;
  Buf->Alloc = Alloc;
}

template <typename t>
void DeallocBufT(typed_buffer<t>* Buf) {
  buffer RawBuf{(byte*)Buf->Data, i64(Buf->Size * sizeof(t)), Buf->Alloc};
  DeallocBuf(&RawBuf);
}

} // namespace mg

#undef mg_HeapArray
#define mg_HeapArray(Name, Type, Size)\
  using namespace mg;\
  typed_buffer<Type> Name;\
  AllocBufT(&Name, (Size));\
  mg_CleanUp(__LINE__, DeallocBufT(&Name))

#undef mg_HeapArray0
#define mg_HeapArray0(Name, Type, Size)\
  using namespace mg;\
  typed_buffer<Type> Name;\
  AllocBufT0(&Name, (Size));\
  mg_CleanUp(__LINE__, DeallocBufT(&Name))

#undef mg_StackHeapArrays
#define mg_StackHeapArrays(Name, Type, StackArraySize, HeapArraySize)\
  using namespace mg;\
  typed_buffer<Type> Name[StackArraySize] = {}; \
  for (int I = 0; I < (StackArraySize); ++I)\
    AllocBufT(&Name[I], (HeapArraySize));\
  mg_CleanUp(__LINE__, {\
    for (int I = 0; I < (StackArraySize); ++I) DeallocBufT(&Name[I]); })

#undef mg_HeapHeapArrays
#define mg_HeapHeapArrays(Name, Type, SizeOuter, SizeInner)\
  using namespace mg;\
  typed_buffer<typed_buffer<Type>> Name;\
  AllocBufT(&Name, (SizeOuter));\
  for (int I = 0; I < (SizeOuter); ++I) \
    AllocBufT(&Name[I], (SizeInner));\
  mg_CleanUp(__LINE__, {\
    for (int I = 0; I < (SizeOuter); ++I) DeallocBufT(&Name[I]);\
    DeallocBufT(&Name);\
  })
