#pragma once

#include <stdint.h>
#include <stdio.h>
#include "mg_assert.h"

namespace mg {

using uint    = unsigned int;
using byte    = uint8_t;
using int8    = int8_t;
using i8      = int8;
using int16   = int16_t;
using i16     = int16;
using int32   = int32_t;
using i32     = int32;
using int64   = int64_t;
using i64     = int64;
using uint8   = uint8_t;
using u8      = uint8;
using uint16  = uint16_t;
using u16     = uint16;
using uint32  = uint32_t;
using u32     = uint32;
using uint64  = uint64_t;
using u64     = uint64;
using float32 = float;
using f32     = float32;
using float64 = double;
using f64     = float64;
using cstr    = const char*;

/* Something to replace std::array */
template <typename t, int N>
struct array;

/* Vector in 2D, supports .X, .UV, and [] */
template <typename t>
struct v2;
/* Vector in 3D, supports .X, .XY, .UV, .RGB and [] */
template <typename t>
struct v3;

struct buffer {
  byte* Data = nullptr;
  size_t Size = 0;
};

} // namespace mg

#include "mg_types.inl"
