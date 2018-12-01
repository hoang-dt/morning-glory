#pragma once

#include "mg_types.h"

namespace mg {

thread_local char ScratchBuffer[1024]; // General purpose buffer for string-related operations

/* Abstract away memory allocations/deallocations */
template <typename t>
bool Allocate(t** Ptr, i64 Size);

template <typename t>
void Deallocate(t** Ptr);

bool AllocateBuffer(buffer* Buf, i64 Size);
void DeallocateBuffer(buffer* Buf);

void MemCopy(buffer* Dst, const buffer& Src);

} // namespace mg

#include "mg_memory.inl"
