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

mg_Inline buffer::
buffer(allocator* AllocIn)
  : Alloc(AllocIn) {}

mg_Inline buffer::
buffer(const byte* DataIn, i64 BytesIn, allocator* AllocIn)
  : Data(const_cast<byte*>(DataIn)), Bytes(BytesIn), Alloc(AllocIn) {}

mg_TAi buffer::
buffer(t (&Arr)[N])
  : Data((byte*)const_cast<t*>(&Arr[0])), Bytes(sizeof(Arr)) {}

mg_Ti(t) buffer::
buffer(const buffer_t<t>& Buf)
  : Data((byte*)const_cast<t*>(Buf.Data))
  , Bytes(Buf.Size * sizeof(t)), Alloc(Buf.Alloc) {}

mg_Inline byte& buffer::
operator[](i64 Idx) const {
  assert(Idx < Bytes);
  return const_cast<byte&>(Data[Idx]);
}

mg_Inline buffer::
operator bool() const { return this->Data && this->Bytes; }

mg_Inline bool
operator==(const buffer& Buf1, const buffer& Buf2) {
  return Buf1.Data == Buf2.Data && Buf1.Bytes == Buf2.Bytes;
}

mg_Inline i64
Size(const buffer& Buf) { return Buf.Bytes; }

mg_Inline void
Resize(buffer* Buf, i64 NewSize) {
  if (Size(*Buf) < NewSize) {
    DeallocBuf(Buf);
    AllocBuf(Buf, NewSize);
  }
}

/* typed_buffer stuffs */
mg_Ti(t) buffer_t<t>::
buffer_t() = default;

mg_T(t) template <int N> mg_Inline buffer_t<t>::
buffer_t(t (&Arr)[N])
  : Data(&Arr[0]), Size(N) {}

mg_Ti(t) buffer_t<t>::
buffer_t(const t* DataIn, i64 SizeIn, allocator* AllocIn)
  : Data(const_cast<t*>(DataIn)), Size(SizeIn), Alloc(AllocIn) {}

mg_Ti(t) buffer_t<t>::
buffer_t(const buffer& Buf)
  : Data((t*)const_cast<byte*>(Buf.Data))
  , Size(Buf.Bytes / sizeof(t)), Alloc(Buf.Alloc) {}

mg_Ti(t) t& buffer_t<t>::
operator[](i64 Idx) const {
  assert(Idx < Size);
  return const_cast<t&>(Data[Idx]);
}

mg_Ti(t) i64
Size(const buffer_t<t>& Buf) { return Buf.Size; }

mg_Ti(t) i64
Bytes(const buffer_t<t>& Buf) { return Buf.Size * sizeof(t); }

mg_Ti(t) buffer_t<t>::
operator bool() const { return Data && Size; }

#undef mg_TA
#undef mg_TAi

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


