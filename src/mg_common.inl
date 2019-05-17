#pragma once

#include <assert.h>
#include <stdio.h>

namespace mg {

mg_Ti(t) auto&
Value(t&& T) {
  if constexpr (is_pointer<typename remove_reference<t>::type>::Value)
    return *T;
  else
    return T;
}

template <>
struct traits<i8> {
  using signed_t   = i8;
  using unsigned_t = u8;
  static constexpr u8 NBinaryMask = 0xaa;
  static constexpr i8 Min = -(1 << 7);
  static constexpr i8 Max = (1 << 7) - 1;
};

template <>
struct traits<u8> {
  using signed_t   = i8;
  using unsigned_t = u8;
  static constexpr u8 NBinaryMask = 0xaa;
  static constexpr u8 Min = 0;
  static constexpr u8 Max = (1 << 8) - 1;
};

template <>
struct traits<i16> {
  using signed_t   = i16;
  using unsigned_t = u16;
  static constexpr u16 NBinaryMask = 0xaaaa;
  static constexpr i16 Min = -(1 << 15);
  static constexpr i16 Max = (1 << 15) - 1;
};

template <>
struct traits<u16> {
  using signed_t   = i16;
  using unsigned_t = u16;
  static constexpr u16 NBinaryMask = 0xaaaa;
  static constexpr u16 Min = 0;
  static constexpr u16 Max = (1 << 16) - 1;
};

template <>
struct traits<i32> {
  using signed_t   = i32;
  using unsigned_t = u32;
  using floating_t = f32;
  static constexpr u32 NBinaryMask = 0xaaaaaaaa;
  static constexpr i32 Min = i32(0x80000000);
  static constexpr i32 Max = 0x7fffffff;
};

template <>
struct traits<u32> {
  using signed_t   = i32;
  using unsigned_t = u32;
  static constexpr u32 NBinaryMask = 0xaaaaaaaa;
  static constexpr u32 Min = 0;
  static constexpr u32 Max = 0xffffffff;
};

template <>
struct traits<i64> {
  using signed_t   = i64;
  using unsigned_t = u64;
  using floating_t = f64;
  static constexpr u64 NBinaryMask = 0xaaaaaaaaaaaaaaaaULL;
  static constexpr i64 Min = 0x8000000000000000ll;
  static constexpr i64 Max = 0x7fffffffffffffffull;
};

template <>
struct traits<u64> {
  using signed_t   = i64;
  using unsigned_t = u64;
  static constexpr u64 NBinaryMask = 0xaaaaaaaaaaaaaaaaULL;
  static constexpr u64 Min = 0;
  static constexpr u64 Max = 0xffffffffffffffffull;
};

template <>
struct traits<f32> {
  using integral_t = i32;
  static constexpr int ExpBits = 8;
  static constexpr int ExpBias = (1 << (ExpBits - 1)) - 1;
  static constexpr f32 Min = -FLT_MAX;
  static constexpr f32 Max =  FLT_MAX;
};

template <>
struct traits<f64> {
  using integral_t = i64;
  static constexpr int ExpBits = 11;
  static constexpr int ExpBias = (1 << (ExpBits - 1)) - 1;
  static constexpr f64 Min = -DBL_MAX;
  static constexpr f64 Max =  DBL_MAX;
};

#define mg_TAi template <typename t, int N> mg_Inline

mg_TAi t& stack_array<t, N>::
operator[](int Idx) { assert(Idx < N); return Arr[Idx]; }
mg_TAi t*
Begin   (stack_array<t, N>& A) { return &A.Arr[0]; }
mg_TAi t*
End     (stack_array<t, N>& A) { return &A.Arr[0] + N; }
mg_TAi t*
RevBegin(stack_array<t, N>& A) { return &A.Arr[0] + (N - 1); }
mg_TAi t*
RevEnd  (stack_array<t, N>& A) { return &A.Arr[0] - 1; }
mg_TAi int
Size(const stack_array<t, N>&) { return N; }


mg_Inline buffer::
buffer() = default;

mg_Inline buffer::
buffer(byte* DataIn, i64 BytesIn, allocator* AllocIn)
  : Data(DataIn), Bytes(BytesIn), Alloc(AllocIn) {}

mg_TAi buffer::
buffer(t (&Arr)[N]) : Data((byte*)&Arr[0]), Bytes(sizeof(Arr)) {}

mg_Ti(t) buffer::
buffer(buffer_t<t> Buf)
  : Data(Buf.Data), Bytes(Buf.Size * sizeof(t)), Alloc(Buf.Alloc) {}

mg_Inline byte& buffer::
operator[](i64 Idx) { assert(Idx < Bytes); return Data[Idx]; }

mg_Inline buffer::
operator bool() const { return this->Data && this->Bytes; }

mg_Inline bool
operator==(const buffer& Buf1, const buffer& Buf2) {
  return Buf1.Data == Buf2.Data && Buf1.Bytes == Buf2.Bytes;
}

/* typed_buffer stuffs */
mg_Ti(t) buffer_t<t>::
buffer_t() = default;

mg_T(t) template <int N> mg_Inline buffer_t<t>::
buffer_t(t (&Arr)[N]) : Data(&Arr[0]), Size(N) {}

mg_Ti(t) buffer_t<t>::
buffer_t(t* DataIn, i64 SizeIn, allocator* AllocIn)
  : Data(DataIn), Size(SizeIn), Alloc(AllocIn) {}

mg_Ti(t) buffer_t<t>::
buffer_t(buffer Buf)
  : Data((t*)Buf.Data), Size(Buf.Bytes / sizeof(t)), Alloc(Buf.Alloc) {}

mg_Ti(t) t& buffer_t<t>::
operator[](i64 Idx) { assert(Idx < Size); return Data[Idx]; }

mg_Ti(t) i64
Size(const buffer_t<t>& Buf) { return Buf.Size; }

mg_Ti(t) i64
Bytes(const buffer_t<t>& Buf) { return Buf.Size * sizeof(t); }

mg_Ti(t) buffer_t<t>::
operator bool() const { return Data && Size; }

#undef mg_TA
#undef mg_TAi

/* v2 stuffs */
mg_Ti(t) v2<t>::
v2() {}
mg_Ti(t) v2<t>::
v2(t V): X(V), Y(V) {}
mg_Ti(t) v2<t>::
v2(t X, t Y): X(X), Y(Y) {}
mg_T(t) mg_Ti(u) v2<t>::
v2(const v2<u>& Other) : X(Other.X), Y(Other.Y) {}
mg_Ti(t) t& v2<t>::
operator[](int Idx) { assert(Idx < 2); return E[Idx]; }
mg_T(t) mg_Ti(u) v2<t>& v2<t>::
operator=(const v2<u>& other) { X = other.X; Y = other.Y; return *this; }

/* v3 stuffs */
mg_Ti(t) v3<t>::
v3() {}
mg_Ti(t) v3<t>::
v3(t V): X(V), Y(V), Z(V) {}
mg_Ti(t) v3<t>::
v3(t X, t Y, t Z): X(X), Y(Y), Z(Z) {}
mg_T(t) mg_Ti(u) v3<t>::
v3(const v3<u>& Other) : X(Other.X), Y(Other.Y), Z(Other.Z) {}
mg_Ti(t) t& v3<t>::
operator[](int Idx) { assert(Idx < 3); return E[Idx]; }
mg_T(t) mg_Ti(u) v3<t>& v3<t>::
operator=(const v3<u>& Rhs) { X = Rhs.X; Y = Rhs.Y; Z = Rhs.Z; return *this; }


// TODO: move the following to mg_macros.h?

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

