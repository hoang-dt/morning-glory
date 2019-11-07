/* Track I/O accesses to a file, in blocks */
#pragma once

#include "mg_common.h"
#include "mg_array.h"

namespace mg {

struct file_io_tracker {
  struct access {
    i64 Where;
    i64 Bytes;
  };
  i64 FileSize;
  int BlockSize; // in bytes (common values: 512, 4096, 8192, ...)
  array<access> Accesses;
};

void Init(file_io_tracker* Tracker);
void Access(file_io_tracker* Tracker, i64 Where, i64 Bytes);
void Dealloc(file_io_tracker* Tracker);

} // namespace mg
