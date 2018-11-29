#pragma once

#include <stdlib.h>

namespace mg {

thread_local char ScratchBuffer[1024]; // General purpose buffer for string-related operations

/* Abstract away memory allocations/deallocations */
// TODO: deal with out-of-memory errors
#define mg_Allocate(Buf, Size) (Buf = (decltype(Buf))malloc(Size * sizeof(*Buf)))
#define mg_Deallocate(Buf) { free(Buf); Buf = nullptr; }

} // namespace mg
