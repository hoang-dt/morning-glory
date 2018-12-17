#pragma once

#include "mg_types.h"

namespace mg {

template <typename t>
struct dynamic_array;
struct Block;

void Encode(const f64* Data, v3i Dims, v3i TileDims, int Bits, const dynamic_array<Block>& Subbands,
  cstr FileName);
void Decode(cstr FileName, v3i Dims, v3i TileDims, int Bits, const dynamic_array<Block>& Subbands,
  f64* Data);

} // namespace mg