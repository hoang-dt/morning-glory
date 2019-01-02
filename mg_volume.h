#pragma once

#include "mg_types.h"

namespace mg {

struct block {
  i64 Pos;
  i64 Size;
};

struct volume {
  buffer Buffer;
  block Block;    
};

struct data_type;
struct error;
/* Read a volume from a file */
error ReadVolume(cstr FileName, v3i Dims, data_type Type, volume* Volume);

/* Copy a region of the first volume to a region of the second volume */
void Copy(); // TODO

} // namespace mg
