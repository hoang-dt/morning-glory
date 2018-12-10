#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_assert.h"
#include "mg_common_types.h"
#include "mg_math.h"
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

array<u8, 8> Orders[4] = {
  { 127, 127, 127, 127, 127, 127, 127, 127 }, // not used
  { 0, 1, 127, 127, 127, 127, 127, 127 }, // for 1D
  { 0, 1, 2, 3, 127, 127, 127, 127 }, // for 2D
  { 0, 1, 2, 4, 3, 5, 6, 7 } // for 3D
};

void BuildSubbands(int NDims, v3i N, int NLevels, dynamic_array<Block>* Subbands) {
  mg_Assert(NDims <= 3);
  mg_Assert(N.Z == 1 || NDims == 3);
  mg_Assert(N.Y == 1 || NDims >= 2);
  const array<u8, 8>& Order = Orders[NDims];
  Reserve(Subbands, ((1 << NDims) - 1) * NLevels + 1);
  v3i M = N;
  for (int I = 0; I < NLevels; ++I) {
    v3i P((M.X + 1) >> 1, (M.Y + 1) >> 1, (M.Z + 1) >> 1);
    for (int J = (1 << NDims) - 1; J > 0; --J) {
      u8 X = Order[J] & 1u, Y = (Order[J] >> 1) & 1u, Z = (Order[J] >> 2) & 1u;
      v3i Sm((X == 0) ? P.X : M.X - P.X,
             (Y == 0) ? P.Y : M.Y - P.Y,
             (Z == 0) ? P.Z : M.Z - P.Z);
      if (Prod<i64>(Sm) != 0) // child exists
        PushBack(Subbands, Block{ XyzToI(N, v3i(X, Y, Z) * P), XyzToI(N, Sm) });
    }
    M = P;
  }
  PushBack(Subbands, Block{ 0, XyzToI(N, M) });
  Reverse(Begin(*Subbands), End(*Subbands));
}

} // namespace mg

#undef TypeChooser
