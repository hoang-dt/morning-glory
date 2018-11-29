#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

namespace mg {

#define mg_NumberTypes\
  int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64

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
using str     = char*;
using cstr    = const char*;

template <typename t>
struct Traits {
  // using signed_t =
  // using unsigned_t =
  // using integral_t =
  // static constexpr uint NegabinaryMask =
  // static constexpr int ExponentBits
  // static constexpr int ExponentBias
};

/* Something to replace std::array */
template <typename t, int N>
struct array;
template <typename t, int N> t* Begin(array<t, N>& A);
template <typename t, int N> const t* ConstBegin(const array<t, N>& A);
template <typename t, int N> t* End(array<t, N>& A);
template <typename t, int N> const t* ConstEnd(const array<t, N>& A);
template <typename t, int N> t* ReverseBegin(array<t, N>& A);
template <typename t, int N> const t* ConstReverseBegin(const array<t, N>& A);
template <typename t, int N> t* ReverseEnd(array<t, N>& A);
template <typename t, int N> const t* ConstReverseEnd(const array<t, N>& A);
template <typename t, int N> int Size(array<t, N>&);

/* Vector in 2D, supports .X, .UV, and [] */
template <typename t>
struct v2;
// using v2i  = v2<i32>;
// using v2u  = v2<u32>;
// using v2l  = v2<i64>;
// using v2ul = v2<u64>;
// using v2f  = v2<f32>;
// using v2d  = v2<f64>;

/* Vector in 3D, supports .X, .XY, .UV, .RGB and [] */
template <typename t>
struct v3;
// using v3i  = v3<i32>;
// using v3u  = v3<u32>;
// using v3l  = v3<i64>;
// using v3ul = v3<u64>;
// using v3f  = v3<f32>;
// using v3d  = v3<f64>;

struct buffer {
  byte* Data = nullptr;
  size_t Size = 0;
};

} // namespace mg

#include "mg_types.inl"
