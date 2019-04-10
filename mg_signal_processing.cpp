#include <math.h>
#include "mg_assert.h"
#include "mg_common_types.h"
#include "mg_math.h"
#include "mg_signal_processing.h"

namespace mg {

/* TODO: rewrite these functions to take in a volume with dimensions, strides, etc */

f64 SquaredError(const byte* F, const byte* G, i64 Size, data_type Type) {
#define Body(type)\
  const type* FPtr = (const type*)F;\
  const type* GPtr = (const type*)G;\
  f64 Err = 0;\
  for (i64 I = 0; I < Size; ++I) {\
    f64 Diff = FPtr[I] - GPtr[I];\
    Err += Diff * Diff;\
  }\
  return Err;

  TypeChooser(Type)
  return 0;
#undef Body
}

f64 SquaredError(const volume& F, const volume& G) {
  printf("%llu %llu", F.Extent.DimsCompact, G.Extent.DimsCompact);
  mg_Assert(Dims(F.Extent) == Dims(G.Extent));
  mg_Assert(F.Type == G.Type);
#define Body(type)\
  const type* FPtr = (type*)F.Buffer.Data;\
  const type* GPtr = (type*)G.Buffer.Data;\
  f64 Err = 0;\
  v3i PosF, PosG;\
  v3i BegF = Pos(F.Extent), BegG = Pos(G.Extent);\
  v3i EndF = BegF + Dims(F.Extent);\
  v3i StrideF = Stride(F.Extent), StrideG = Stride(G.Extent);\
  v3i BigDimsF = BigDims(F), BigDimsG = BigDims(G);\
  mg_BeginFor3Lockstep(PosF, BegF, EndF, StrideF, PosG, BegG, EndG, StrideG) {\
    i64 I = XyzToI(BigDimsF, PosF);\
    i64 J = XyzToI(BigDimsG, PosG);\
    f64 Diff = f64(FPtr[I]) - f64(GPtr[J]);\
    Err += Diff * Diff;\
  }\
  mg_EndFor3\
  return Err;

  TypeChooser(F.Type)
  return 0;
#undef Body
}

f64 RMSError(const byte* F, const byte* G, i64 Size, data_type Type) {
  return sqrt(SquaredError(F, G, Size, Type) / Size);
}

f64 PSNR(const byte* F, const byte* G, i64 Size, data_type Type) {
#define Body(type)\
  const type* FPtr = (const type*)F;\
  f64 Err = SquaredError(F, G, Size, Type);\
  auto MinMax = MinMaxElement(FPtr, FPtr + Size);\
  f64 D = 0.5 * (*(MinMax.Max) - *(MinMax.Min));\
  Err /= Size;\
  return 20.0 * log10(D) - 10.0 * log10(Err);

  TypeChooser(Type)
  return 0;
#undef Body
}

//f64 PSNR(const volume& F, const volume& G) {
//  mg_Assert(F.Type == G.Type);
//  mg_Assert(SmallDims(F) == SmallDims(G));
//#define Body(type)\
//  type MinVal = Traits<type>::Max;\
//  type MaxVal = Traits<type>::Min;\
//  f64 Err
//  auto FIt = ConstBegin(F);
//  auto GIt = ConstBegin(G);
//  auto FEnd = ConstEnd(F);
//  while (FIt != FEnd) {
//
//  }
//  i64 N = Prod<i64>(SmallDims(F));
//  for (i64 I = 0; I < N; ++I) {
//
//  }
//#undef Body
//}

void ConvertToNegabinary(const byte* FIn, i64 Size, byte* FOut, data_type Type) {
#define Body(type)\
  using utype = typename Traits<type>::unsigned_t;\
  const type* FInPtr  = (const type*)FIn;\
  utype* FOutPtr = (utype*)FOut;\
  for (i64 I = 0; I < Size; ++I) {\
    auto Mask = Traits<type>::NegabinaryMask;\
    FOutPtr[I] = utype((FInPtr[I] + Mask) ^ Mask);\
  }

  TypeChooserInt(Type)
#undef Body
}

void ConvertFromNegabinary(const byte* FIn, i64 Size, byte* FOut, data_type Type) {
#define Body(type)\
  using utype = typename Traits<type>::unsigned_t;\
  const utype* FInPtr  = (const utype*)FIn;\
  type* FOutPtr = (type*)FOut;\
  for (i64 I = 0; I < Size; ++I) {\
    auto Mask = Traits<type>::NegabinaryMask;\
    FOutPtr[I] = type((FInPtr[I] ^ Mask) - Mask);\
  }

  TypeChooserInt(Type)
#undef Body
}

int Quantize(const byte* FIn, i64 Size, int Bits, byte* FOut, data_type Type) {
#define Body(type)\
  using itype = typename Traits<type>::integral_t;\
  const type* FInPtr = (const type*)FIn;\
  itype* FOutPtr = (itype*)FOut;\
  type Max = *(MaxElement(FInPtr, FInPtr + Size,\
                          [](auto A, auto B) { return fabs(A) < fabs(B); }));\
  int EMax = Exponent(fabs(Max));\
  f64 Scale = ldexp(1, Bits - 1 - EMax);\
  for (i64 I = 0; I < Size; ++I)\
    FOutPtr[I] = itype(Scale * FInPtr[I]);\
  return EMax;

  TypeChooserFloat(Type)
  return 0;
#undef Body
}

void Dequantize(const byte* FIn, i64 Size, int EMax, int Bits, byte* FOut, data_type Type) {
#define Body(type)\
  using itype = typename Traits<type>::integral_t;\
  const itype* FInPtr = (const itype*)FIn;\
  type* FOutPtr = (type*)FOut;\
  f64 Scale = 1.0 / ldexp(1, Bits - 1 - EMax);\
  for (i64 I = 0; I < Size; ++I)\
    FOutPtr[I] = type(Scale * FInPtr[I]);

  TypeChooserFloat(Type)
#undef Body
}

} // namespace mg

