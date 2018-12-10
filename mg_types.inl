#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

namespace mg {

template <>
struct Traits<i8> {
  using signed_t   = i8;
  using unsigned_t = u8;
  static constexpr u8 NegabinaryMask = 0xaa;
};

template <>
struct Traits<u8> {
  using signed_t   = i8;
  using unsigned_t = u8;
  static constexpr u8 NegabinaryMask = 0xaa;
};

template <>
struct Traits<i16> {
  using signed_t   = i16;
  using unsigned_t = u16;
  static constexpr u16 NegabinaryMask = 0xaaaa;
};

template <>
struct Traits<u16> {
  using signed_t   = i16;
  using unsigned_t = u16;
  static constexpr u16 NegabinaryMask = 0xaaaa;
};

template <>
struct Traits<i32> {
  using signed_t   = i32;
  using unsigned_t = u32;
  using floating_t = f32;
  static constexpr u32 NegabinaryMask = 0xaaaaaaaa;
};

template <>
struct Traits<u32> {
  using signed_t   = i32;
  using unsigned_t = u32;
  static constexpr u32 NegabinaryMask = 0xaaaaaaaa;
};

template <>
struct Traits<i64> {
  using signed_t   = i64;
  using unsigned_t = u64;
  using floating_t = f64;
  static constexpr u64 NegabinaryMask = 0xaaaaaaaaaaaaaaaaULL;
};

template <>
struct Traits<u64> {
  using signed_t   = i64;
  using unsigned_t = u64;
  static constexpr u64 NegabinaryMask = 0xaaaaaaaaaaaaaaaaULL;
};

template <>
struct Traits<f32> {
  using integral_t = i32;
  static constexpr int ExponentBits = 8;
  static constexpr int ExponentBias = (1 << (ExponentBits - 1)) - 1;
};

template <>
struct Traits<f64> {
  using integral_t = i64;
  static constexpr int ExponentBits = 11;
  static constexpr int ExponentBias = (1 << (ExponentBits - 1)) - 1;
};

/* Something to replace std::array */
#define TemplateArr template <typename t, int N>
TemplateArr t& array<t, N>::operator[](int Idx) { assert(Idx < N); return Arr[Idx]; }
TemplateArr const t& array<t, N>::operator[](int Idx) const { assert(Idx < N); return Arr[Idx]; }
TemplateArr t* Begin(array<t, N>& A) { return &A.Arr[0]; }
TemplateArr const t* ConstBegin(const array<t, N>& A) { return &A.Arr[0]; }
TemplateArr t* End(array<t, N>& A) { return &A.Arr[0] + N; }
TemplateArr const t* ConstEnd(const array<t, N>& A) { return &A.Arr[0] + N; }
TemplateArr t* ReverseBegin(array<t, N>& A) { return &A.Arr[0] + (N - 1); }
TemplateArr const t* ConstReverseBegin(const array<t, N>& A) { return &A.Arr[0] + (N - 1); }
TemplateArr t* ReverseEnd(array<t, N>& A) { return &A.Arr[0] - 1; }
TemplateArr t* ConstReverseEnd(const array<t, N>& A) { return &A.Arr[0] - 1; }
TemplateArr int Size(array<t, N>&) { return N; }
#undef TemplateArr

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
template <typename t>
struct v2 {
  union {
    struct { t X, Y; };
    struct { t U, V; };
    t E[2];
  };
  v2() = default;
  v2(t X, t Y): X(X), Y(Y) {}
  template <typename u>
  v2(v2<u> Other): X(Other.X), Y(Other.Y) {}
  t& operator[](int Idx) { assert(Idx < 2); return E[Idx]; }
  t operator[](int Idx) const { assert(Idx < 2); return E[Idx]; }
  template <typename u>
  v2& operator=(v2<u> other) { X = other.X; Y = other.Y; return *this; }
};
using v2i  = v2<i32>;
using v2u  = v2<u32>;
using v2l  = v2<i64>;
using v2ul = v2<u64>;
using v2f  = v2<f32>;
using v2d  = v2<f64>;

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
  v3() = default;
  v3(t X, t Y, t Z): X(X), Y(Y), Z(Z) {}
  template <typename u>
  v3(v3<u> Other): X(Other.X), Y(Other.Y), Z(Other.Z) {}
  t& operator[](int Idx) { assert(Idx < 3); return E[Idx]; }
  t operator[](int Idx) const { assert(Idx < 3); return E[Idx]; }
  template <typename u>
  v3& operator=(v3<u> other) { X = other.X; Y = other.Y; Z = other.Z; return *this; }
};
using v3i  = v3<i32>;
using v3u  = v3<u32>;
using v3l  = v3<i64>;
using v3ul = v3<u64>;
using v3f  = v3<f32>;
using v3d  = v3<f64>;
#pragma GCC diagnostic pop

} // namespace mg
