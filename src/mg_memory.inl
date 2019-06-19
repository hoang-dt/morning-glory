#pragma once

#include "mg_scopeguard.h"

namespace mg {

mg_T(t) void
AllocTypedBuf(buffer_t<t>* Buf, i64 Size, allocator* Alloc) {
  buffer RawBuf;
  AllocBuf(&RawBuf, i64(Size * sizeof(t)), Alloc);
  Buf->Data = (t*)RawBuf.Data;
  Buf->Size = Size;
  Buf->Alloc = Alloc;
}

mg_T(t) void
AllocTypedBuf0(buffer_t<t>* Buf, i64 Size, allocator* Alloc) {
  buffer RawBuf;
  AllocBuf0(&RawBuf, i64(Size * sizeof(t)), Alloc);
  Buf->Data = (t*)RawBuf.Data;
  Buf->Size = Size;
  Buf->Alloc = Alloc;
}

mg_T(t) void
DeallocTypedBuf(buffer_t<t>* Buf) {
  buffer RawBuf{(byte*)Buf->Data, i64(Buf->Size * sizeof(t)), Buf->Alloc};
  DeallocBuf(&RawBuf);
}

} // namespace mg

#undef mg_MallocArray
#define mg_MallocArray(Name, Type, Size)\
  using namespace mg;\
  buffer_t<Type> Name;\
  AllocTypedBuf(&Name, (Size));\
  mg_CleanUp(__LINE__, DeallocTypedBuf(&Name))

#undef mg_MallocArray0
#define mg_MallocArray0(Name, Type, Size)\
  using namespace mg;\
  buffer_t<Type> Name;\
  AllocTypedBuf0(&Name, (Size));\
  mg_CleanUp(__LINE__, DeallocTypedBuf(&Name))

#undef mg_ArrayOfMallocArrays
#define mg_ArrayOfMallocArrays(Name, Type, SizeOuter, SizeInner)\
  using namespace mg;\
  buffer_t<Type> Name[SizeOuter] = {}; \
  for (int I = 0; I < (SizeOuter); ++I)\
    AllocTypedBuf(&Name[I], (SizeInner));\
  mg_CleanUp(__LINE__, {\
    for (int I = 0; I < (SizeOuter); ++I) DeallocTypedBuf(&Name[I]); })

#undef mg_MallocArrayOfArrays
#define mg_MallocArrayOfArrays(Name, Type, SizeOuter, SizeInner)\
  using namespace mg;\
  buffer_t<buffer_t<Type>> Name;\
  AllocTypedBuf(&Name, (SizeOuter));\
  for (int I = 0; I < (SizeOuter); ++I) \
    AllocTypedBuf(&Name[I], (SizeInner));\
  mg_CleanUp(__LINE__, {\
    for (int I = 0; I < (SizeOuter); ++I) DeallocTypedBuf(&Name[I]);\
    DeallocTypedBuf(&Name);\
  })
