#include "mg_assert.h"
#include "mg_common_types.h"
#include "mg_types.h"
#include "mg_wavelet.h"

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

namespace mg {

void Cdf53Forward(f64* F, v3l Dimensions, int NLevels, data_type Type) {
#define Body(type)\
  type* FPtr = (type*)F;\
  for (int I = 0; I < NLevels; ++I) {\
    ForwardLiftCdf53X(FPtr, Dimensions, v3l{I, I, I});\
    ForwardLiftCdf53Y(FPtr, Dimensions, v3l{I, I, I});\
    ForwardLiftCdf53Z(FPtr, Dimensions, v3l{I, I, I});\
  }\

  TypeChooser(Type)
#undef Body
}

void Cdf53Inverse(f64* F, v3l Dimensions, int NLevels, data_type Type) {
#define Body(type)\
  type* FPtr = (type*)F;\
  for (int I = NLevels - 1; I >= 0; --I) {\
    InverseLiftCdf53Z(FPtr, Dimensions, v3l{I, I, I});\
    InverseLiftCdf53Y(FPtr, Dimensions, v3l{I, I, I});\
    InverseLiftCdf53X(FPtr, Dimensions, v3l{I, I, I});\
  }\

  TypeChooser(Type)
#undef Body
}

} // namespace mg

#undef TypeChooser
