#pragma once

#include <stdlib.h>

namespace mg {

/* Abstract away memory allocations/deallocations */
#define mg_Allocate(buf, size) (buf = (decltype(buf))malloc(size))
#define mg_Deallocate(buf) { free(buf); buf = nullptr; }

} // namespace mg
