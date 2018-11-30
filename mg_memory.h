#pragma once

#include "mg_types.h"

namespace mg {

thread_local char ScratchBuffer[1024]; // General purpose buffer for string-related operations

/* Abstract away memory allocations/deallocations */
// TODO: encapsulate memcpy to check for errors
template <typename t>
bool Allocate(t** Buf, i64 Size);

template <typename t>
void Deallocate(t** Buf);

} // namespace mg

#include "mg_memory.inl"
