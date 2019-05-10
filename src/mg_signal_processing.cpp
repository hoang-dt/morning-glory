#include <math.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_signal_processing.h"
#include "mg_volume.h"

namespace mg {

f64
SqError(const grid_volume& F, const grid_volume& G) {
  mg_Assert(Dims(F) == Dims(G));
  mg_Assert(F.Base.Type == G.Base.Type);
#define Body(type)\
  buffer_t<type> FBuf(F.Base.Buffer);\
  buffer_t<type> GBuf(G.Base.Buffer);\
  f64 Err = 0;\
  mg_BeginGridLoop2(F, G) {\
    f64 Diff = f64(FBuf[I]) - f64(GBuf[J]);\
    Err += Diff * Diff;\
  } mg_EndGridLoop2\
  return Err;

  mg_DispatchOnType(F.Base.Type)
  return 0;
#undef Body
}

f64
RMSError(const grid_volume& F, const grid_volume& G) {
  return sqrt(SqError(F, G) / Size(F));
}

f64
PSNR(const grid_volume& F, const grid_volume& G) {
  mg_Assert(Dims(F) == Dims(G));
  mg_Assert(F.Base.Type == G.Base.Type);
#define Body(type)\
  buffer_t<type> FBuf(F.Base.Buffer);\
  buffer_t<type> GBuf(G.Base.Buffer);\
  f64 Err = 0;\
  type MinElem = traits<type>::Max;\
  type MaxElem = traits<type>::Min;\
  mg_BeginGridLoop2(F, G) {\
    f64 Diff = f64(FBuf[I]) - f64(GBuf[J]);\
    Err += Diff * Diff;\
    MinElem = Min(MinElem ,FBuf[I]);\
    MaxElem = Max(MaxElem, FBuf[I]);\
  } mg_EndGridLoop2\
  f64 D = 0.5 * (MaxElem - MinElem);\
  Err = Err / Size(F);\
  return 20.0 * log10(D) - 10.0 * log10(Err);

  mg_DispatchOnType(F.Base.Type)
  return 0;
#undef Body
}

void
FwdNegaBinary(const grid_volume& Src, grid_volume* Dst) {
  mg_Assert(Dims(Src) <= Dims(*Dst));
  mg_Assert(Dst->Base.Type == UnsignedType(Src.Base.Type));
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  buffer_t<type>  SrcBuf(Src.Base.Buffer);\
  buffer_t<utype> DstBuf(Dst->Base.Buffer);\
  auto Mask = traits<type>::NBinaryMask;\
  mg_BeginGridLoop2(Src, *Dst) {\
    DstBuf[J] = utype((SrcBuf[I] + Mask) ^ Mask);\
  } mg_EndGridLoop2\

  mg_DispatchOnInt(Src.Base.Type)
#undef Body
}

void
InvNegaBinary(const grid_volume& Src, grid_volume* Dst) {
  mg_Assert(Dims(Src) <= Dims(*Dst));
  mg_Assert(Src.Base.Type == UnsignedType(Dst->Base.Type));
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  buffer_t<utype> SrcBuf(Src.Base.Buffer);\
  buffer_t<type>  DstBuf(Dst->Base.Buffer);\
  auto Mask = traits<type>::NBinaryMask;\
  mg_BeginGridLoop2(Src, *Dst) {\
    DstBuf[J] = type((SrcBuf[I] ^ Mask) - Mask);\
  } mg_EndGridLoop2\

  mg_DispatchOnInt(Dst->Base.Type)
#undef Body
}

int
Quantize(const grid_volume& Src, grid_volume* Dst, int Bits) {
  mg_Assert(Dims(Src) <= Dims(*Dst));
  mg_Assert(Dst->Base.Type == IntType(Src.Base.Type));
#define Body(type)\
  using itype = typename traits<type>::integral_t;\
  buffer_t<type> SrcBuf(Src.Base.Buffer);\
  buffer_t<itype>  DstBuf(Dst->Base.Buffer);\
  /* find the max absolute value */\
  type MaxAbs = 0;\
  mg_BeginGridLoop(Src) {\
    MaxAbs = Max(MaxAbs, (type)fabs(SrcBuf[I]));\
  } mg_EndGridLoop\
  int EMax = Exponent(fabs(MaxAbs));\
  /* quantize */\
  f64 Scale = ldexp(1, Bits - 1 - EMax);\
  mg_BeginGridLoop2(Src, *Dst) {\
    DstBuf[J] = itype(Scale * SrcBuf[I]);\
  } mg_EndGridLoop2\
  return EMax;

  mg_DispatchOnFloat(Src.Base.Type)
  return 0;
#undef Body
}

void
Dequantize(const grid_volume& Src, int EMax, int Bits, grid_volume* Dst) {
  mg_Assert(Dims(Src) <= Dims(*Dst));
  mg_Assert(Src.Base.Type == IntType(Dst->Base.Type));
#define Body(type)\
  using itype = typename traits<type>::integral_t;\
  buffer_t<itype> SrcBuf(Src.Base.Buffer);\
  buffer_t<type>  DstBuf(Dst->Base.Buffer);\
  f64 Scale = 1.0 / ldexp(1, Bits - 1 - EMax);\
  mg_BeginGridLoop2(Src, *Dst) {\
    DstBuf[J] = (Scale * SrcBuf[I]);\
  } mg_EndGridLoop2

  mg_DispatchOnFloat(Dst->Base.Type)
#undef Body
}

} // namespace mg

