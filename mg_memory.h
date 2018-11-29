#pragma once

#include <stdlib.h>

namespace mg {

thread_local char ScratchBuffer[1024]; // General purpose buffer for string-related operations

/* Abstract away memory allocations/deallocations */
// TODO: deal with out-of-memory errors
#define mg_Allocate(buf, size) (buf = (decltype(buf))malloc(size))
#define mg_Deallocate(buf) { free(buf); buf = nullptr; }

} // namespace mg
