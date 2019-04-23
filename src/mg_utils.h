#pragma once

#include "mg_types.h"

namespace mg {

u32 Murmur3_32(const u8* Key, int Len, u32 Seed);

}
