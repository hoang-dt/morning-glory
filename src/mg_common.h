#pragma once

#include <inttypes.h>
#include <float.h>
#include <stdint.h>
#include "mg_macros.h"

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

mg_T(t)
struct traits {
  // using signed_t =
  // using unsigned_t =
  // using integral_t =
  // static constexpr uint NBinaryMask =
  // static constexpr int ExpBits
  // static constexpr int ExpBias
};

struct true_type { static constexpr bool Value = true; };
struct false_type { static constexpr bool Value = false; };
mg_T(t) struct remove_const { typedef t type; };
mg_T(t) struct remove_const<const t> { typedef t type; };
mg_T(t) struct remove_volatile { typedef t type; };
mg_T(t) struct remove_volatile<volatile t> { typedef t type; };
mg_T(t) struct remove_cv { typedef typename remove_volatile<typename remove_const<t>::type>::type type; };
mg_T(t) struct remove_reference { typedef t type; };
mg_T(t) struct remove_reference<t&> { typedef t type; };
mg_T(t) struct remove_reference<t&&> { typedef t type; };
mg_T2(t1, t2) struct is_same_type : false_type {};
mg_T(t) struct is_same_type<t, t> : true_type {};
mg_T(t) struct is_pointer_helper : false_type {};
mg_T(t) struct is_pointer_helper<t*> : true_type {};
mg_T(t) struct is_pointer : is_pointer_helper<typename remove_cv<t>::type> {};
mg_T(t) auto& Value(t&& T);

/* Something to replace std::array */
template <typename t, int N>
struct stack_array {
  static_assert(N > 0);
  t Arr[N];
  t& operator[](int Idx);
};

#define mg_TA template <typename t, int N>

mg_TA t* Begin(stack_array<t, N>& A);
mg_TA t* End(stack_array<t, N>& A);
mg_TA t* RevBegin(stack_array<t, N>& A);
mg_TA t* RevEnd(stack_array<t, N>& A);
mg_TA int Size(const stack_array<t, N>& A);

#undef mg_TA

/* Vector in 2D, supports .X, .UV, and [] */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
mg_T(t)
struct v2 {
  union {
    struct { t X, Y; };
    struct { t U, V; };
    t E[2];
  };
  inline static const v2 Zero = v2(0);
  inline static const v2 One = v2(1);
  v2();
  explicit v2(t V);
  v2(t X, t Y);
  mg_T(u) v2(const v2<u>& Other);
  t& operator[](int Idx);
  mg_T(u) v2& operator=(const v2<u>& Rhs);
};
using v2i  = v2<i32>;
using v2u  = v2<u32>;
using v2l  = v2<i64>;
using v2ul = v2<u64>;
using v2f  = v2<f32>;
using v2d  = v2<f64>;

/* Vector in 3D, supports .X, .XY, .UV, .RGB and [] */
mg_T(t)
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
  inline static const v3 Zero = v3(0);
  inline static const v3 One = v3(1);
  v3();
  explicit v3(t V);
  v3(t X, t Y, t Z);
  mg_T(u) v3(const v3<u>& Other);
  t& operator[](int Idx);
  mg_T(u) v3& operator=(const v3<u>& Rhs);
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

mg_T(t) struct typed_buffer;
struct allocator;
struct buffer {
  byte* Data = nullptr;
  i64 Bytes = 0;
  allocator* Alloc = nullptr;
  buffer();
  template <typename t, int N>
  buffer(t (&Arr)[N]);
  buffer(byte* DataIn, i64 BytesIn, allocator* AllocIn = nullptr);
  mg_T(t) buffer(typed_buffer<t> Buf);
  byte& operator[](i64 Idx);
  explicit operator bool() const;
};

bool operator==(const buffer& Buf1, const buffer& Buf2);

mg_T(t)
struct typed_buffer {
  t* Data = nullptr;
  i64 Size = 0;
  allocator* Alloc = nullptr;
  typed_buffer();
  template <int N>
  typed_buffer(t (&Arr)[N]);
  typed_buffer(t* DataIn, i64 SizeIn, allocator* AllocIn = nullptr);
  typed_buffer(buffer Buf);
  t& operator[](i64 Idx);
  explicit operator bool() const;
};

mg_T(t) i64 Size(const typed_buffer<t>& Buf);
mg_T(t) i64 Bytes(const typed_buffer<t>& Buf);

} // namespace mg

#include "mg_common.inl"
