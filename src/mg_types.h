#pragma once

#include "mg_macros.h"
#include <inttypes.h>
#include <float.h>
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

#define CloneFunc(type) mg_ForceInline void Clone(type* B, type A) { *B = A; }
CloneFunc(i8)
CloneFunc(u8)
CloneFunc(i16)
CloneFunc(u16)
CloneFunc(i32)
CloneFunc(u32)
CloneFunc(i64)
CloneFunc(u64)
CloneFunc(f32)
CloneFunc(f64)

template <typename t>
struct Traits {
  // using signed_t =
  // using unsigned_t =
  // using integral_t =
  // static constexpr uint NegabinaryMask =
  // static constexpr int ExpBits
  // static constexpr int ExpBias
};

template <typename T1, typename T2>
struct IsSameType{ enum { Result = false }; };
template< typename T>
struct IsSameType<T, T> { enum { Result = true }; };

/* Something to replace std::array */
template <typename t, int N>
struct array {
  static_assert(N > 0);
  t Arr[N];
  t& operator[](int Idx);
  const t& operator[](int Idx) const;
};
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
template <typename t>
struct v2 {
  union {
    struct { t X, Y; };
    struct { t U, V; };
    t E[2];
  };
  static v2 Zero();
  static v2 One();
  v2();
  explicit v2(t V);
  v2(t X, t Y);
  template <typename u> v2(v2<u> Other);
  t& operator[](int Idx);
  t operator[](int Idx) const;
  template <typename u> v2& operator=(v2<u> other);
};
using v2i  = v2<i32>;
using v2u  = v2<u32>;
using v2l  = v2<i64>;
using v2ul = v2<u64>;
using v2f  = v2<f32>;
using v2d  = v2<f64>;

/* Vector in 3D, supports .X, .XY, .UV, .RGB and [] */
template <typename t>
struct v3 {
  union {
    struct { t X, Y, Z; };
    struct { t U, V, __; };
    struct { t R, G, B; };
    struct { v2<t> XY; t Ignored0_; };
    struct { t Ignored1_; v2<t> YZ; };
    struct { v2<t> UV; t Ignored2_; };
    struct { t Ignored3_; v2<t> V__; };
    t E[3];
  };
  static v3 Zero();
  static v3 One();
  v3();
  explicit v3(t V);
  v3(t X, t Y, t Z);
  template <typename u> v3(v3<u> Other);
  t& operator[](int Idx);
  t operator[](int Idx) const;
  template <typename u> v3& operator=(v3<u> other);
};
#pragma GCC diagnostic pop
using v3i  = v3<i32>;
using v3u  = v3<u32>;
using v3l  = v3<i64>;
using v3ul = v3<u64>;
using v3f  = v3<f32>;
using v3d  = v3<f64>;

/* 3-level nested for loop */
#define mg_BeginFor3(Counter, Begin, End, Step)
#define mg_EndFor3
#define mg_BeginFor3Lockstep(C1, B1, E1, S1, C2, B2, E2, S2)

template <typename t>
struct typed_buffer;

struct allocator;
struct buffer {
  byte* Data = nullptr;
  i64 Bytes = 0;
  allocator* Alloc = nullptr;
  buffer();
  buffer(byte* Data, i64 Bytes, allocator* Alloc);
  template<typename t> buffer(typed_buffer<t> Buf);
  byte& operator[](i64 Idx);
  byte operator[](i64 Idx) const;
};

template <typename t>
struct typed_buffer {
  t* Data = nullptr;
  i64 Size = 0;
  allocator* Alloc = nullptr;
  typed_buffer();
  typed_buffer(t* Data, i64 Size, allocator* Alloc);
  typed_buffer(buffer Buf);
  t& operator[](i64 Idx);
  const t& operator[](i64 Idx) const;
};
template <typename t>
i64 Size(typed_buffer<t> Buf);
template <typename t>
i64 Bytes(typed_buffer<t> Buf);

} // namespace mg

#include "mg_types.inl"
