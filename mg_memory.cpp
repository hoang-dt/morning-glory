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

} // namespace mg
