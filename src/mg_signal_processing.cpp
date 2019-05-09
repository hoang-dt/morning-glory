#include <math.h>
#include "mg_assert.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_signal_processing.h"

namespace mg {

f64
SqError(const grid<volume>& F, const grid<volume>& G) {
  mg_Assert(Dims(F) == Dims(G));
  mg_Assert(F.Base.Type == G.Base.Type);
#define Body(type)\
  typed_buffer<type> FBuf(F.Base.Buffer);\
  typed_buffer<type> GBuf(G.Base.Buffer);\
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
RMSError(const grid<volume>& F, const grid<volume>& G) {
  return sqrt(SquaredError(F, G) / Size(F));
}

f64
PSNR(const grid<volume>& F, const grid<volume>& G) {
  mg_Assert(Dims(F) == Dims(G));
  mg_Assert(F.Base.Type == G.Base.Type);
#define Body(type)\
  typed_buffer<type> FBuf(F.Base.Buffer);\
  typed_buffer<type> GBuf(G.Base.Buffer);\
  f64 Err = 0;\
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
ToNegaBinary(grid<volume>* Dst, const grid<volume>& Src) {
  mg_Assert(Dims(Src) <= Dims(*Dst));
  mg_Assert(Dst->Base.Type == UnsignedType(Src.Base.Type));
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  typed_buffer<type> SrcBuf(Src.Base.Buffer);\
  typed_buffer<utype> DstBuf(Dst->Base.Buffer);\
  mg_BeginGridLoop2(Src, *Dst) {\
    auto Mask = traits<type>::NBinaryMask;\
    DstBuf[J] = utype((SrcBuf[I] + Mask) ^ Mask);\
  } mg_EndGridLoop2\

  mg_DispatchOnInt(Src.Base.Type)
#undef Body
}

void
FromNegaBinary(grid<volume>* Dst, const grid<volume>& Src) {
  mg_Assert(Dims(Src) <= Dims(*Dst));
  mg_Assert(Src.Base.Type == UnsignedType(Dst->Base.Type));
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  typed_buffer<utype> SrcBuf(Src.Base.Buffer);\
  typed_buffer<type> DstBuf(Dst->Base.Buffer);\
  mg_BeginGridLoop2(Src, *Dst) {\
    auto Mask = traits<type>::NBinaryMask;\
    DstBuf[J] = type((SrcBuf[I] ^ Mask) - Mask);\
  } mg_EndGridLoop2\

  mg_DispatchOnInt(Dst->Base.Type)
#undef Body
}

int
Quantize(grid<volume>* Dst, const grid<volume>& Src, int Bits) {
#define Body(type)\
  using itype = typename traits<type>::integral_t;\
  type* FInPtr = (type*)FIn;\
  itype* FOutPtr = (itype*)FOut;\
  type Max = *(MaxElem(FInPtr, FInPtr + Size,\
                       [](auto A, auto B) { return fabs(A) < fabs(B); }));\
  int EMax = Exponent(fabs(Max));\
  f64 Scale = ldexp(1, Bits - 1 - EMax);\
  for (i64 I = 0; I < Size; ++I)\
    FOutPtr[I] = itype(Scale * FInPtr[I]);\
  return EMax;

  mg_DispatchOnFloat(Type)
  return 0;
#undef Body
}

void
Dequantize(byte* FIn, i64 Size, int EMax, int Bits, byte* FOut, dtype Type) {
#define Body(type)\
  using itype = typename traits<type>::integral_t;\
  itype* FInPtr = (itype*)FIn;\
  type* FOutPtr = (type*)FOut;\
  f64 Scale = 1.0 / ldexp(1, Bits - 1 - EMax);\
  for (i64 I = 0; I < Size; ++I)\
    FOutPtr[I] = type(Scale * FInPtr[I]);

  mg_DispatchOnFloat(Type)
#undef Body
}

} // namespace mg

