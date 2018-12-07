#include <math.h>
#include "mg_assert.h"
#include "mg_math.h"
#include "mg_signal_processing.h"

#define TypeChooser(Type)\
  if (Type == mg::data_type::float64) {\
    Body(f64)\
  } else if (Type == mg::data_type::float32) {\
    Body(f32)\
  } else if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else if (Type == mg::data_type::int16) {\
    Body(i16)\
  } else if (Type == mg::data_type::int8) {\
    Body(i8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#define TypeChooserInt(Type)\
  if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else if (Type == mg::data_type::int16) {\
    Body(i16)\
  } else if (Type == mg::data_type::int8) {\
    Body(i8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#define TypeChooserInt32And64(Type)\
  if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#define TypeChooserFloat(Type)\
  if (Type == mg::data_type::float64) {\
    Body(f64)\
  } else if (Type == mg::data_type::float32) {\
    Body(f32)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

namespace mg {

f64 SquaredError(const f64* F, const f64* G, i64 Size, data_type Type) {
#define Body(type)\
  const type* FPtr = (const type*)F;\
  const type* GPtr = (const type*)G;\
  \
  type Err = 0;\
  for (i64 I = 0; I < Size; ++I) {\
    type Diff = FPtr[I] - GPtr[I];\
    Err += Diff * Diff;\
  }\
  return Err;

  TypeChooser(Type)
  return 0;
#undef Body
}

f64 RMSError(const f64* F, const f64* G, i64 Size, data_type Type) {
  return sqrt(SquaredError(F, G, Size, Type) / Size);
}

f64 PSNR(const f64* F, const f64* G, i64 Size, data_type Type) {
  f64 Err = SquaredError(F, G, Size, Type);
  auto MinMax = MinMaxElement(F, F + Size);
  f64 D = 0.5 * (*(MinMax.Max) - *(MinMax.Min));
  Err /= Size;
  return 20.0 * log10(D) - 10.0 * log10(Err);
}

void ConvertToNegabinary(const i64* FIn, i64 Size, u64* FOut, data_type Type) {
#define Body(type)\
  using utype = typename Traits<type>::unsigned_t;\
  const type* FInPtr  = (const type*)FIn;\
  utype* FOutPtr = (utype*)FOut;\
  \
  for (i64 I = 0; I < Size; ++I) {\
    auto Mask = Traits<type>::NegabinaryMask;\
    FOutPtr[I] = utype((FInPtr[I] + Mask) ^ Mask);\
  }

  TypeChooserInt(Type)
#undef Body
}

void ConvertFromNegabinary(const u64* FIn, i64 Size, i64* FOut, data_type Type) {
#define Body(type)\
  using utype = typename Traits<type>::unsigned_t;\
  const utype* FInPtr  = (const utype*)FIn;\
  type* FOutPtr = (type*)FOut;\
  \
  for (i64 I = 0; I < Size; ++I) {\
    auto Mask = Traits<type>::NegabinaryMask;\
    FOutPtr[I] = type((FInPtr[I] ^ Mask) - Mask);\
  }

  TypeChooserInt(Type)
#undef Body
}

int Quantize(const f64* FIn, i64 Size, int Bits, i64* FOut, data_type Type) {
#define Body(type)\
  using itype = typename Traits<type>::integral_t;\
  const type* FInPtr = (const type*)FIn;\
  itype* FOutPtr = (itype*)FOut;\
  \
  type Max = *(MaxElement(FIn, FIn + Size, [](auto A, auto B) { return fabs(A) < fabs(B); }));\
  int EMax = Exponent(fabs(Max));\
  double Scale = ldexp(1, Bits - 1 - EMax);\
  for (i64 I = 0; I < Size; ++I)\
    FOutPtr[I] = itype(Scale * FInPtr[I]);\
  return EMax;

  TypeChooserFloat(Type)
  return 0;
#undef Body
}

void Dequantize(const i64* FIn, i64 Size, int EMax, int Bits, f64* FOut, data_type Type) {
#define Body(type)\
  using itype = typename Traits<type>::integral_t;\
  const itype* FInPtr = (const itype*)FIn;\
  type* FOutPtr = (type*)FOut;\
  \
  double Scale = 1.0 / ldexp(1, Bits - EMax);\
  for (i64 I = 0; I < Size; ++I)\
    FOutPtr[I] = type(Scale * FInPtr[I]);

  TypeChooserFloat(Type)
#undef Body
}

} // namespace mg

#undef TypeChooser
#undef TypeChooserInt
#undef TypeChooserInt32And64
#undef TypeChooserFloat