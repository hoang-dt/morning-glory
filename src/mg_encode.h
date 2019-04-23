#pragma once

#include "mg_types.h"

namespace mg {

template <typename t>
struct dynamic_array;
struct block_bounds_complete;

void Encode(const f64* Data, v3i Dims, v3i TileDims, int Bits, f64 Tolerance,
            const dynamic_array<block_bounds_complete>& Subbands, cstr FileName);
void Decode(cstr FileName, v3i Dims, v3i TileDims, int Bits, f64 Tolerance,
            const dynamic_array<block_bounds_complete>& Subbands, f64* Data);

} // namespace mg
