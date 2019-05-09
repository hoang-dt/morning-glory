#pragma once

#include "mg_enum.h"
#include "mg_macros.h"
#include "mg_common.h"

mg_Enum(dtype, int,
  int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64)

/*
Dispatch some code depending on Type. To use, define a Body macro which
contains the code to run. Presumably the code makes use of Type. */
#define mg_DispatchOnType(Type)
#define mg_DispatchOnInt(Type)
#define mg_DispatchOnFloat(Type)\

namespace mg {

int SizeOf(dtype Type);
int BitSizeOf(dtype Type);
dtype IntType(dtype InputType);
dtype UnsignedType(dtype InputType);
mg_T(t) bool MatchTypes(dtype Type);

} // namespace mg

#include "mg_data_types.inl"
