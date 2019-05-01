#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "mg_macros.h"

namespace mg {

template <>
struct Traits<i8> {
  using signed_t   = i8;
  using unsigned_t = u8;
  static constexpr u8 NegabinaryMask = 0xaa;
  static constexpr i8 Min = -(1 << 7);
  static constexpr i8 Max = (1 << 7) - 1;
};

template <>
struct Traits<u8> {
  using signed_t   = i8;
  using unsigned_t = u8;
  static constexpr u8 NegabinaryMask = 0xaa;
  static constexpr u8 Min = 0;
  static constexpr u8 Max = (1 << 8) - 1;
};

template <>
struct Traits<i16> {
  using signed_t   = i16;
  using unsigned_t = u16;
  static constexpr u16 NegabinaryMask = 0xaaaa;
  static constexpr i16 Min = -(1 << 15);
  static constexpr i16 Max = (1 << 15) - 1;
};

template <>
struct Traits<u16> {
  using signed_t   = i16;
  using unsigned_t = u16;
  static constexpr u16 NegabinaryMask = 0xaaaa;
  static constexpr u16 Min = 0;
  static constexpr u16 Max = (1 << 16) - 1;
};

template <>
struct Traits<i32> {
  using signed_t   = i32;
  using unsigned_t = u32;
  using floating_t = f32;
  static constexpr u32 NegabinaryMask = 0xaaaaaaaa;
  static constexpr i32 Min = i32(0x80000000);
  static constexpr i32 Max = 0x7fffffff;
};

template <>
struct Traits<u32> {
  using signed_t   = i32;
  using unsigned_t = u32;
  static constexpr u32 NegabinaryMask = 0xaaaaaaaa;
  static constexpr u32 Min = 0;
  static constexpr u32 Max = 0xffffffff;
};

template <>
struct Traits<i64> {
  using signed_t   = i64;
  using unsigned_t = u64;
  using floating_t = f64;
  static constexpr u64 NegabinaryMask = 0xaaaaaaaaaaaaaaaaULL;
  static constexpr i64 Min = 0x8000000000000000ll;
  static constexpr i64 Max = 0x7fffffffffffffffull;
};

template <>
struct Traits<u64> {
  using signed_t   = i64;
  using unsigned_t = u64;
  static constexpr u64 NegabinaryMask = 0xaaaaaaaaaaaaaaaaULL;
  static constexpr u64 Min = 0;
  static constexpr u64 Max = 0xffffffffffffffffull;
};

template <>
struct Traits<f32> {
  using integral_t = i32;
  static constexpr int ExpBits = 8;
  static constexpr int ExpBias = (1 << (ExpBits - 1)) - 1;
  static constexpr f32 Min = -FLT_MAX;
  static constexpr f32 Max = FLT_MAX;
};

template <>
struct Traits<f64> {
  using integral_t = i64;
  static constexpr int ExpBits = 11;
  static constexpr int ExpBias = (1 << (ExpBits - 1)) - 1;
  static constexpr f64 Min = -DBL_MAX;
  static constexpr f64 Max = DBL_MAX;
};

/* Something to replace std::array */
#define TemplateArr template <typename t, int N>
TemplateArr mg_ForceInline t& array<t, N>::operator[](int Idx) {
  assert(Idx < N);
  return Arr[Idx];
}
TemplateArr mg_ForceInline const t& array<t, N>::operator[](int Idx) const {
  assert(Idx < N);
  return Arr[Idx];
}
TemplateArr mg_ForceInline t* Begin(array<t, N>& A) {
  return &A.Arr[0];
}
TemplateArr mg_ForceInline const t* ConstBegin(const array<t, N>& A) {
  return &A.Arr[0];
}
TemplateArr mg_ForceInline t* End(array<t, N>& A) {
  return &A.Arr[0] + N;
}
TemplateArr mg_ForceInline const t* ConstEnd(const array<t, N>& A) {
  return &A.Arr[0] + N;
}
TemplateArr mg_ForceInline t* ReverseBegin(array<t, N>& A) {
  return &A.Arr[0] + (N - 1);
}
TemplateArr mg_ForceInline const t* ConstReverseBegin(const array<t, N>& A) {
  return &A.Arr[0] + (N - 1);
}
TemplateArr mg_ForceInline t* ReverseEnd(array<t, N>& A) {
  return &A.Arr[0] - 1;
}
TemplateArr mg_ForceInline t* ConstReverseEnd(const array<t, N>& A) {
  return &A.Arr[0] - 1;
}
TemplateArr mg_ForceInline int Size(array<t, N>&) {
  return N;
}
#undef TemplateArr

mg_ForceInline buffer::buffer() = default;
mg_ForceInline buffer::buffer(byte* DataIn, i64 BytesIn, allocator* AllocIn)
  : Data(DataIn), Bytes(BytesIn), Alloc(AllocIn) {}
template <typename t, int N> mg_ForceInline
buffer::buffer(t (&Arr)[N]) : Data((byte*)&Arr[0]), Bytes(sizeof(Arr)) {}
template<typename t> mg_ForceInline
buffer::buffer(const typed_buffer<t>& Buf)
  : Data(Buf.Data), Bytes(Buf.Size * sizeof(t)) {}

mg_ForceInline
byte& buffer::operator[](i64 Idx) {
  assert(Idx < Bytes);
  return Data[Idx];
}

mg_ForceInline
byte buffer::operator[](i64 Idx) const {
  assert(Idx < Bytes);
  return Data[Idx];
}

/* typed_buffer stuffs */
template <typename t> mg_ForceInline
typed_buffer<t>::typed_buffer() = default;

template <typename t> template <int N> mg_ForceInline
typed_buffer<t>::typed_buffer(t (&Arr)[N]) : Data(&Arr[0]), Size(N) {}

template <typename t> mg_ForceInline
typed_buffer<t>::typed_buffer(t* DataIn, i64 SizeIn, allocator* AllocIn)
  : Data(DataIn), Size(SizeIn), Alloc(AllocIn) {}

template <typename t> mg_ForceInline
typed_buffer<t>::typed_buffer(const buffer& Buf)
  : Data((t*)Buf.Data), Size(Buf.Bytes / sizeof(t)) {}

template <typename t> mg_ForceInline
t& typed_buffer<t>::operator[](i64 Idx) {
  assert(Idx < Size); return Data[Idx];
}
template <typename t> mg_ForceInline
const t& typed_buffer<t>::operator[](i64 Idx) const {
  assert(Idx < Size);
  return Data[Idx];
}

template <typename t> mg_ForceInline
i64 Size(const typed_buffer<t>& Buf) {
  return Buf.Size;
}
template <typename t> mg_ForceInline
i64 Bytes(const typed_buffer<t>& Buf) {
  return Buf.Size * sizeof(t);
}

/* v2 stuffs */
template <typename t> mg_ForceInline
v2<t> v2<t>::Zero() {
  static v2<t> Z(0);
  return Z;
}
template <typename t> mg_ForceInline
v2<t> v2<t>::One() {
  static v2<t> O(1);
  return O;
}

template <typename t> mg_ForceInline
v2<t>::v2() {}
template <typename t> mg_ForceInline
v2<t>::v2(t V): X(V), Y(V) {}
template <typename t> mg_ForceInline
v2<t>::v2(t X, t Y): X(X), Y(Y) {}
template <typename t> template <typename u> mg_ForceInline
v2<t>::v2(const v2<u>& Other): X(Other.X), Y(Other.Y) {}

template <typename t> mg_ForceInline
t& v2<t>::operator[](int Idx) {
  assert(Idx < 2);
  return E[Idx];
}
template <typename t> mg_ForceInline
t v2<t>::operator[](int Idx) const {
  assert(Idx < 2);
  return E[Idx];
}

template <typename t> template <typename u> mg_ForceInline
v2<t>& v2<t>::operator=(const v2<u>& other) {
  X = other.X;
  Y = other.Y;
  return *this;
}

/* v3 stuffs */
template <typename t> mg_ForceInline
v3<t> v3<t>::Zero() {
  static v3<t> Z(0);
  return Z;
}
template <typename t> mg_ForceInline
v3<t> v3<t>::One() {
  static v3<t> O(1);
  return O;
}

template <typename t> mg_ForceInline
v3<t>::v3() {}
template <typename t> mg_ForceInline
v3<t>::v3(t V): X(V), Y(V), Z(V) {}
template <typename t> mg_ForceInline
v3<t>::v3(t X, t Y, t Z): X(X), Y(Y), Z(Z) {}
template <typename t> template <typename u> mg_ForceInline
v3<t>::v3(const v3<u>& Other): X(Other.X), Y(Other.Y), Z(Other.Z) {}

template <typename t> mg_ForceInline
t& v3<t>::operator[](int Idx) {
  assert(Idx < 3);
  return E[Idx];
}
template <typename t> mg_ForceInline
t v3<t>::operator[](int Idx) const {
  assert(Idx < 3);
  return E[Idx];
}
template <typename t> template <typename u> mg_ForceInline
v3<t>& v3<t>::operator=(const v3<u>& other) {
  X = other.X;
  Y = other.Y;
  Z = other.Z;
  return *this;
}

#undef mg_BeginFor3
#define mg_BeginFor3(Counter, Begin, End, Step)\
  for (Counter.Z = (Begin).Z; Counter.Z < (End).Z; Counter.Z += (Step).Z) {\
  for (Counter.Y = (Begin).Y; Counter.Y < (End).Y; Counter.Y += (Step).Y) {\
  for (Counter.X = (Begin).X; Counter.X < (End).X; Counter.X += (Step).X)

#undef mg_EndFor3
#define mg_EndFor3 }}

#undef mg_BeginFor3Lockstep
#define mg_BeginFor3Lockstep(C1, B1, E1, S1, C2, B2, E2, S2)\
  for (C1.Z = (B1).Z, C2.Z = (B2).Z; C1.Z < (E1).Z; C1.Z += (S1).Z, C2.Z += (S2).Z) {\
  for (C1.Y = (B1).Y, C2.Y = (B2).Y; C1.Y < (E1).Y; C1.Y += (S1).Y, C2.Y += (S2).Y) {\
  for (C1.X = (B1).X, C2.X = (B2).X; C1.X < (E1).X; C1.X += (S1).X, C2.X += (S2).X)

} // namespace mg

