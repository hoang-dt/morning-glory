#pragma once

#include "mg_enum.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_memory.h"
#include "mg_string.h"
#include "mg_types.h"

mg_Enum(data_type, int,
  int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64)

namespace mg {

/* In string form:
file = C:/My Data/my file.raw
name = combustion
field = o2
dimensions = 512 512 256
data type = float32 */
struct metadata {
  char File[256] = "";
  char Name[32] = "";
  char Field[32] = "";
  union { v3i Dimensions = { 0, 0, 0 }; v3i Dims; };
  data_type DataType = data_type::__Invalid__;
  inline thread_local static char String[384];
}; // struct metadata

cstr ToString(metadata& Meta);
error ReadMetadata(cstr Fname, metadata* Meta);

} // namespace mg
