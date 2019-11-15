#include <math.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_signal_processing.h"
#include "mg_volume.h"

namespace mg {

// TODO: move the templated functions into an .inl file
// TODO: think of a way that does not require writing a function three times

mg_TT(t1, t2) f64
SqError(const buffer_t<t1>& FBuf, const buffer_t<t2>& GBuf) {
  mg_Assert(FBuf.Size == GBuf.Size);
  extent Ext(v3i(FBuf.Size, 1, 1));
  return SqError(Ext, volume(FBuf), Ext, volume(GBuf));
}

mg_TT(t1, t2) f64
SqError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol) {
  mg_Assert(Dims(FGrid) <= Dims(FVol));
  mg_Assert(Dims(GGrid) <= Dims(GVol));
  mg_Assert(Dims(FGrid) == Dims(GGrid));
#define Body(type1, type2)\
  auto FIt = Begin<type1>(FGrid, FVol), FEnd = End<type1>(FGrid, FVol);\
  auto GIt = Begin<type2>(GGrid, GVol);\
  f64 Err = 0;\
  for (; FIt != FEnd; ++FIt, ++GIt) {\
    f64 Diff = f64(*FIt) - f64(*GIt);\
    Err += Diff * Diff;\
  }\
  return Err;

  mg_DispatchOn2Types(FVol.Type, GVol.Type)
  return 0;
#undef Body
}

f64
SqError(const volume& FVol, const volume& GVol) {
  return SqError(extent(FVol), FVol, extent(GVol), GVol);
}

mg_TT(t1, t2) f64
RMSError(const buffer_t<t1>& FBuf, const buffer_t<t2>& GBuf) {
  mg_Assert(FBuf.Size == GBuf.Size);
  extent Ext(v3i(SBuf.Size, 1, 1));
  return RMSError(Ext, volume(FBuf), Ext, volume(GBuf));
}

mg_TT(t1, t2) f64
RMSError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol) {
  return sqrt(SqError(FGrid, FVol, GGrid, GVol) / Size(FGrid));
}

f64
RMSError(const volume& FVol, const volume& GVol) {
  return RMSError(extent(FVol), FVol, extent(GVol), GVol);
}

mg_TT(t1, t2) f64
PSNR(const buffer_t<t1>& FBuf, const buffer_t<t2>& GBuf) {
  mg_Assert(FBuf.Size == GBuf.Size);
  extent Ext(v3i(SBuf.Size, 1, 1));
  return PSNR(Ext, volume(FBuf), Ext, volume(GBuf));
}

mg_TT(t1, t2) f64
PSNR(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol) {
  mg_Assert(Dims(FGrid) <= Dims(FVol));
  mg_Assert(Dims(GGrid) <= Dims(GVol));
  mg_Assert(Dims(FGrid) == Dims(GGrid));
#define Body(type1, type2)\
  auto FIt = Begin<type1>(FGrid, FVol), FEnd = End<type1>(FGrid, FVol);\
  auto GIt = Begin<type2>(GGrid, GVol);\
  f64 Err = 0;\
  type1 MinElem = traits<type1>::Max, MaxElem = traits<type1>::Min;\
  for (; FIt != FEnd; ++FIt, ++GIt) {\
    f64 Diff = f64(*FIt) - f64(*GIt);\
    Err += Diff * Diff;\
    MinElem = Min(MinElem, *FIt);\
    MaxElem = Max(MaxElem, *FIt);\
  }\
  f64 D = 0.5 * (MaxElem - MinElem);\
  Err = Err / Size(FGrid);\
  return 20.0 * log10(D) - 10.0 * log10(Err);

  mg_DispatchOn2Types(FVol.Type, GVol.Type)
  return 0;
#undef Body
}

f64
PSNR(const volume& FVol, const volume& GVol) {
  return PSNR(extent(FVol), FVol, extent(GVol), GVol);
}

mg_TT(t, u) void
FwdNegaBinary(const buffer_t<t>& SBuf, buffer_t<u>* DBuf) {
  mg_Assert(is_signed<t>::Value);
  mg_Assert(is_same_type<typename traits<t>::unsigned_t, u>::Value);
  if (!(*DBuf))
    AllocTypedBuf(DBuf, SBuf.Size);
  mg_Assert(SBuf.Size == DBuf->Size);
  extent Ext(v3i(SBuf.Size, 1, 1));
  volume DVol(*DBuf);
  return FwdNegaBinary(Ext, volume(SBuf), Ext, &DVol);
}

mg_TT(t1, t2) void
FwdNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), UnsignedType(SVol.Type));
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(IsSigned(SVol.Type));
  mg_Assert(DVol->Type == UnsignedType(SVol.Type));
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  auto SIt = Begin<type>(SGrid, SVol), SEnd = End<type>(SGrid, SVol);\
  auto DIt = Begin<utype>(DGrid, *DVol);\
  auto Mask = traits<type>::NBinaryMask;\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = utype((*SIt + Mask) ^ Mask);\

  mg_DispatchOnInt(SVol.Type)
#undef Body
}

void
FwdNegaBinary(const volume& SVol, volume* DVol) {
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), UnsignedType(SVol.Type));
  return FwdNegaBinary(extent(SVol), SVol, extent(*DVol), DVol);
}

mg_TT(t, u) void
InvNegaBinary(const buffer_t<t>& SBuf, buffer_t<u>* DBuf) {
  mg_Assert(is_signed<u>::Value);
  mg_Assert(is_same_type<typename traits<u>::unsigned_t, t>::Value);
  if (!(*DBuf))
    AllocTypedBuf(DBuf, SBuf.Size);
  mg_Assert(SBuf.Size == DBuf->Size);
  extent Ext(v3i(SBuf.Size, 1, 1));
  volume DVol(*DBuf);
  return InvNegaBinary(Ext, volume(SBuf), Ext, &DVol);
}

mg_TT(t1, t2) void
InvNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), SignedType(SVol.Type));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(IsSigned(DVol->Type));
  mg_Assert(SVol.Type == UnsignedType(DVol->Type));
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  auto SIt = Begin<utype>(SGrid, SVol), SEnd = End<utype>(SGrid, SVol);\
  auto DIt = Begin<type>(DGrid, *DVol);\
  auto Mask = traits<type>::NBinaryMask;\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = type((*SIt ^ Mask) - Mask);\

  mg_DispatchOnInt(DVol->Type)
#undef Body
}

void
InvNegaBinary(const volume& SVol, volume* DVol) {
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), SignedType(SVol.Type));
  return InvNegaBinary(extent(SVol), SVol, extent(*DVol), DVol);
}

mg_TT(t, u) int
Quantize(int Bits, const buffer_t<t>& SBuf, buffer_t<u>* DBuf) {
  mg_Assert(is_floating_point<t>::Value);
  mg_Assert(is_integral<u>::Value);
  mg_Assert(mg_BitSizeOf(t) >= Bits);
  mg_Assert(mg_BitSizeOf(u) >= Bits);
  if (!(*DBuf))
    AllocTypedBuf(DBuf, SBuf.Size);
  mg_Assert(SBuf.Size == DBuf->Size);
  extent Ext(v3i(SBuf.Size, 1, 1));
  volume DVol(*DBuf);
  return Quantize(Bits, Ext, volume(SBuf), Ext, &DVol);
}

mg_TT(t1, t2) int
Quantize(int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), IntType(SVol.Type));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(IsFloatingPoint(SVol.Type));
  mg_Assert(IsIntegral(DVol->Type));
  mg_Assert(BitSizeOf(SVol.Type) >= Bits);
  mg_Assert(BitSizeOf(DVol->Type) >= Bits);
#define Body(type1, type2)\
  auto SIt = Begin<type1>(SGrid, SVol), SEnd = End<type1>(SGrid, SVol);\
  auto DIt = Begin<type2>(DGrid, *DVol);\
  /* find the max absolute value */\
  type1 MaxAbs = 0;\
  for (; SIt != SEnd; ++SIt)\
    MaxAbs = Max(MaxAbs, (type1)fabs(*SIt));\
  int EMax = Exponent(MaxAbs);\
  /* quantize */\
  f64 Scale = ldexp(1, Bits - 1 - EMax);\
  SIt = Begin<type1>(SGrid, SVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = type2(Scale * *SIt);\
  return EMax;

  mg_DispatchOnFloat1(SVol.Type, DVol->Type)
  return 0;
#undef Body
}

int
Quantize(int Bits, const volume& SVol, volume* DVol) {
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), IntType(SVol.Type));
  return Quantize(Bits, extent(SVol), SVol, extent(*DVol), DVol);
}

mg_TT(t, u) void
Dequantize(int EMax, int Bits, const buffer_t<t>& SBuf, buffer_t<u>* DBuf) {
  //mg_Assert((is_same_type<typename traits<u>::integral_t, t>::Value));

  mg_Assert(is_integral<t>::Value);
  mg_Assert(is_floating_point<u>::Value);
  mg_Assert(mg_BitSizeOf(t) >= Bits);
  mg_Assert(mg_BitSizeOf(u) >= Bits);
  if (!(*DBuf))
    AllocTypedBuf(DBuf, SBuf.Size);
  mg_Assert(SBuf.Size == DBuf->Size);
  extent Ext(v3i(SBuf.Size, 1, 1));
  volume DVol(*DBuf);
  return Dequantize(EMax, Bits, Ext, volume(SBuf), Ext, &DVol);
}

mg_TT(t1, t2) void
Dequantize(int EMax, int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), FloatType(SVol.Type));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(BitSizeOf(SVol.Type) >= Bits);
  mg_Assert(BitSizeOf(DVol->Type) >= Bits);
  mg_Assert(IsIntegral(SVol.Type));
  mg_Assert(IsFloatingPoint(DVol->Type));
#define Body(type1, type2)\
  auto SIt = Begin<type1>(SGrid, SVol), SEnd = End<type1>(SGrid, SVol);\
  auto DIt = Begin<type2>(DGrid, *DVol);\
  f64 Scale = 1.0 / ldexp(1, Bits - 1 - EMax);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = (Scale * *SIt);\

  mg_DispatchOnFloat2(SVol.Type, DVol->Type)
#undef Body
}

void
Dequantize(int EMax, int Bits, const volume& SVol, volume* DVol) {
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), FloatType(SVol.Type));
  return Dequantize(EMax, Bits, extent(SVol), SVol, extent(*DVol), DVol);
}

mg_TT(t1, t2) void
ConvertType(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol) {
  mg_Assert(DVol->Type != dtype::__Invalid__);
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), DVol->Type);
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(SVol.Type != DVol->Type);
#define Body(type1, type2)\
  auto SIt = Begin<type1>(SGrid, SVol), SEnd = End<type1>(SGrid, SVol);\
  auto DIt = Begin<type2>(DGrid, *DVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = (type2)(*SIt);\

  mg_DispatchOn2Types(SVol.Type, DVol->Type)
#undef Body
}

void
ConvertType(const volume& SVol, volume* DVol) {
  mg_Assert(DVol->Type != dtype::__Invalid__);
  if (!DVol->Buffer)
    *DVol = volume(Dims(SVol), DVol->Type);
  return ConvertType(extent(SVol), SVol, extent(*DVol), DVol);
}

// TODO: receive one container
mg_T(t) f64
Norm(const t& Begin, const t& End) {
  f64 Result = 0;
  for (auto It = Begin; It != End; ++It)
    Result += (*It) * (*It);
  return sqrt(Result);
}

// TODO: use concept to constraint Input and Output
mg_T(c) void
Upsample(const c& In, c* Out) {
  i64 N = Size(In);
  i64 M = N * 2 - 1;
  Resize(Out, M);
  (*Out)[M - 1] = In[(M - 1) >> 1];
  for (i64 I = M - 3; I >= 0; I -= 2) {
    (*Out)[I    ] = In[I >> 1];
    (*Out)[I + 1] = 0;
  }
}

/* Compute H = F * G */
mg_T(c) void
Convolve(const c& F, const c& G, c* H) {
  i64 N = Size(F), M = Size(G);
  i64 P = N + M - 1;
  Resize(H, P);
  for (i64 I = 0; I < P; ++I) {
    using type = typename remove_cv_ref<decltype(F[0])>::type;
    type Acc = 0;
    i64 K = Min(N - 1, I);
    i64 L = Max(i64(0), I - M + 1);
    for (i64 J = L; J <= K; ++J)
      Acc += F[J] * G[I - J];
    (*H)[I] = Acc;
  }
}

} // namespace mg

