#pragma once

#include <stdint.h>
#include <stdio.h>
#include "mg_assert.h"

namespace mg {

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

struct buffer {
  byte* Data = nullptr;
  size_t Size = 0;
};
/* Something to replace std::array */
#define TemplateArr template <typename t, int N>
TemplateArr
struct array {
  t Arr[N];
  t& operator[](int Idx) { mg_Assert(Idx < N); return Arr[Idx]; }
};
TemplateArr t* Begin(array<t, N>& A) { return &A.Arr[0]; }
TemplateArr t* End(array<t, N>& A) { return &A.Arr[0] + N; }
TemplateArr int Size(array<t, N>&) { return N; }
#undef TemplateArr

}
