#include <math.h>
#include "mg_assert.h"
#include "mg_data_types.h"
#include "mg_math.h"
#include "mg_signal_processing.h"

namespace mg {

/* TODO: rewrite these functions to take in a volume with dimensions, strides, etc */

f64
SquaredError(byte* F, byte* G, i64 Size, data_type Type) {
#define Body(type)\
  type* FPtr = (type*)F;\
  type* GPtr = (type*)G;\
  f64 Err = 0;\
  for (i64 I = 0; I < Size; ++I) {\
    f64 Diff = FPtr[I] - GPtr[I];\
    Err += Diff * Diff;\
  }\
  return Err;

  mg_DispatchOnType(Type)
  return 0;
#undef Body
}

f64
SquaredError(volume& F, volume& G) {
  mg_Assert(Dims(F.Extent) == Dims(G.Extent));
  mg_Assert(F.Type == G.Type);
#define Body(type)\
  type* FPtr = (type*)F.Buffer.Data;\
  type* GPtr = (type*)G.Buffer.Data;\
  f64 Err = 0;\
  v3i PosF, PosG;\
  v3i BegF = Pos(F.Extent), BegG = Pos(G.Extent);\
  v3i EndF = BegF + Dims(F.Extent);\
  v3i StrideF = Strides(F.Extent), StrideG = Strides(G.Extent);\
  v3i BigDimsF = BigDims(F), BigDimsG = BigDims(G);\
  mg_BeginFor3Lockstep(PosF, BegF, EndF, StrideF, PosG, BegG, EndG, StrideG) {\
    i64 I = Row(BigDimsF, PosF);\
    i64 J = Row(BigDimsG, PosG);\
    f64 Diff = f64(FPtr[I]) - f64(GPtr[J]);\
    Err += Diff * Diff;\
  }\
  mg_EndFor3\
  return Err;

  mg_DispatchOnType(F.Type)
  return 0;
#undef Body
}

f64
RMSError(byte* F, byte* G, i64 Size, data_type Type) {
  return sqrt(SquaredError(F, G, Size, Type) / Size);
}

f64
RMSError(volume& F, volume& G) {
  return sqrt(SquaredError(F, G) / Size(F));
}

f64
PSNR(byte* F, byte* G, i64 Size, data_type Type) {
#define Body(type)\
  type* FPtr = (type*)F;\
  f64 Err = SquaredError(F, G, Size, Type);\
  auto MinMax = MinMaxElement(FPtr, FPtr + Size);\
  f64 D = 0.5 * (*(MinMax.Max) - *(MinMax.Min));\
  Err /= Size;\
  return 20.0 * log10(D) - 10.0 * log10(Err);

  mg_DispatchOnType(Type)
  return 0;
#undef Body
}

f64
PSNR(volume& F, volume& G) {
  mg_Assert(Dims(F.Extent) == Dims(G.Extent));
  mg_Assert(F.Type == G.Type);
#define Body(type)\
  type* FPtr = (type*)F.Buffer.Data;\
  type* GPtr = (type*)G.Buffer.Data;\
  f64 Err = 0;\
  v3i PosF, PosG;\
  v3i BegF = Pos(F.Extent), BegG = Pos(G.Extent);\
  v3i EndF = BegF + Dims(F.Extent);\
  v3i StrideF = Strides(F.Extent), StrideG = Strides(G.Extent);\
  v3i BigDimsF = BigDims(F), BigDimsG = BigDims(G);\
  type MinElem = traits<type>::Max;\
  type MaxElem = traits<type>::Min;\
  mg_BeginFor3Lockstep(PosF, BegF, EndF, StrideF, PosG, BegG, EndG, StrideG) {\
    i64 I = Row(BigDimsF, PosF);\
    i64 J = Row(BigDimsG, PosG);\
    f64 Diff = f64(FPtr[I]) - f64(GPtr[J]);\
    Err += Diff * Diff;\
    MinElem = Min(MinElem ,FPtr[I]);\
    MaxElem = Max(MaxElem, FPtr[I]);\
  }\
  mg_EndFor3\
  f64 D = 0.5 * (MaxElem - MinElem);\
  Err = Err / Size(F);\
  return 20.0 * log10(D) - 10.0 * log10(Err);

  mg_DispatchOnType(F.Type)
  return 0;
#undef Body
}

void
ConvertToNegabinary(byte* FIn, i64 Size, byte* FOut, data_type Type) {
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  type* FInPtr  = (type*)FIn;\
  utype* FOutPtr = (utype*)FOut;\
  for (i64 I = 0; I < Size; ++I) {\
    auto Mask = traits<type>::NegabinaryMask;\
    FOutPtr[I] = utype((FInPtr[I] + Mask) ^ Mask);\
  }

  mg_DispatchOnInt(Type)
#undef Body
}

void
ConvertFromNegabinary(byte* FIn, i64 Size, byte* FOut, data_type Type) {
#define Body(type)\
  using utype = typename traits<type>::unsigned_t;\
  utype* FInPtr  = (utype*)FIn;\
  type* FOutPtr = (type*)FOut;\
  for (i64 I = 0; I < Size; ++I) {\
    auto Mask = traits<type>::NegabinaryMask;\
    FOutPtr[I] = type((FInPtr[I] ^ Mask) - Mask);\
  }

  mg_DispatchOnInt(Type)
#undef Body
}

int
Quantize(byte* FIn, i64 Size, int Bits, byte* FOut, data_type Type) {
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
Dequantize(byte* FIn, i64 Size, int EMax, int Bits, byte* FOut, data_type Type) {
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

