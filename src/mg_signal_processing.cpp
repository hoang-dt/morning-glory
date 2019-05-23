#include <math.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_signal_processing.h"
#include "mg_volume.h"

namespace mg {

mg_T2(t1, t2) f64
SqError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol)
{
  mg_Assert(Dims(FGrid) <= Dims(FVol));
  mg_Assert(Dims(GGrid) <= Dims(GVol));
  mg_Assert(Dims(FGrid) == Dims(GGrid));
  mg_Assert(FVol.Type == GVol.Type);
#define Body(type)\
  auto FIt = Begin<type>(FGrid, FVol), FEnd = End<type>(FGrid, FVol);\
  auto GIt = Begin<type>(GGrid, GVol);\
  f64 Err = 0;\
  for (; FIt != FEnd; ++FIt, ++GIt) {\
    f64 Diff = f64(*FIt) - f64(*GIt);\
    Err += Diff * Diff;\
  }\
  return Err;

  mg_DispatchOnType(FVol.Type)
  return 0;
#undef Body
}

mg_T2(t1, t2) f64
RMSError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol)
{
  return sqrt(SqError(FGrid, FVol, GGrid, GVol) / Size(FGrid));
}

mg_T2(t1, t2) f64
PSNR(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol) {
  mg_Assert(Dims(FGrid) <= Dims(FVol));
  mg_Assert(Dims(GGrid) <= Dims(GVol));
  mg_Assert(Dims(FGrid) == Dims(GGrid));
  mg_Assert(FVol.Type == GVol.Type);
#define Body(type)\
  auto FIt = Begin<type>(FGrid, FVol), FEnd = End<type>(FGrid, FVol);\
  auto GIt = Begin<type>(GGrid, GVol);\
  f64 Err = 0;\
  type MinElem = traits<type>::Max, MaxElem = traits<type>::Min;\
  for (; FIt != FEnd; ++FIt, ++GIt) {\
    f64 Diff = f64(*FIt) - f64(*GIt);\
    Err += Diff * Diff;\
    MinElem = Min(MinElem, *FIt);\
    MaxElem = Max(MaxElem, *FIt);\
  }\
  f64 D = 0.5 * (MaxElem - MinElem);\
  Err = Err / Size(FGrid);\
  return 20.0 * log10(D) - 10.0 * log10(Err);

  mg_DispatchOnType(FVol.Type)
  return 0;
#undef Body
}

mg_T2(t1, t2) void
FwdNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol)
{
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
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

mg_T2(t1, t2) void
InvNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol)
{
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
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

mg_T2(t1, t2) int
Quantize(int Bits, const t1& SGrid, const volume& SVol,
         const t2& DGrid, volume* DVol)
{
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(DVol->Type == IntType(SVol.Type));
#define Body(type)\
  using itype = typename traits<type>::integral_t;\
  auto SIt = Begin<type>(SGrid, SVol), SEnd = End<type>(SGrid, SVol);\
  auto DIt = Begin<itype>(DGrid, *DVol);\
  /* find the max absolute value */\
  type MaxAbs = 0;\
  for (; SIt != SEnd; ++SIt)\
    MaxAbs = Max(MaxAbs, (type)fabs(*SIt));\
  int EMax = Exponent(MaxAbs);\
  /* quantize */\
  f64 Scale = ldexp(1, Bits - 1 - EMax);\
  SIt = Begin<type>(SGrid, SVol);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = itype(Scale * *SIt);\
  return EMax;

  mg_DispatchOnFloat(SVol.Type)
  return 0;
#undef Body
}

mg_T2(t1, t2) void
Dequantize(int EMax, int Bits, const t1& SGrid, const volume& SVol,
           const t2& DGrid, volume* DVol)
{
  mg_Assert(Dims(SGrid) <= Dims(SVol));
  mg_Assert(Dims(DGrid) <= Dims(*DVol));
  mg_Assert(Dims(SGrid) == Dims(DGrid));
  mg_Assert(SVol.Type == IntType(DVol->Type));
#define Body(type)\
  using itype = typename traits<type>::integral_t;\
  auto SIt = Begin<itype>(SGrid, SVol), SEnd = End<itype>(SGrid, SVol);\
  auto DIt = Begin<type>(DGrid, *DVol);\
  f64 Scale = 1.0 / ldexp(1, Bits - 1 - EMax);\
  for (; SIt != SEnd; ++SIt, ++DIt)\
    *DIt = (Scale * *SIt);\

  mg_DispatchOnFloat(DVol->Type)
#undef Body
}

} // namespace mg

