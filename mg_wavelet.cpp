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

array<u8, 8> Orders[4] = {
  { 127, 127, 127, 127, 127, 127, 127, 127 }, // not used
  { 0, 1, 127, 127, 127, 127, 127, 127 }, // for 1D
  { 0, 1, 2, 3, 127, 127, 127, 127 }, // for 2D
  { 0, 1, 2, 4, 3, 5, 6, 7 } // for 3D
};

struct Block {
  i64 Pos;
  i64 Size;
};

// void BuildSubbands(int NDims, v3i N, int NLevels, Block* I) {
//   mg_Assert(NDims <= 3);
//   mg_Assert(N.Z == 1 || NDims == 3);
//   mg_Assert(N.Y == 1 || NDims >= 2);
//   const array<u8, 8>& Order = Orders[NDims];
//   I->reserve(((1<<D)-1)*nlevels+1);
//   int3 m = n;
//   for (int i = 0; i < nlevels; ++i) {
//     int3 p((m.x+1)>>1, (m.y+1)>>1, (m.z+1)>>1);
//     for (int j = (1<<D)-1; j > 0; --j) {
//       uint8_t x = order[j]&1u, y = (order[j]>>1)&1u, z = (order[j]>>2)&1u;
//       int3 sm((x==0)?p.x:m.x-p.x, (y==0)?p.y:m.y-p.y, (z==0)?p.z:m.z-p.z);
//       Set sb ={ xyz2i<int64_t>(n, x*p.x, y*p.y, z*p.z), xyz2i<int64_t>(n, sm.x, sm.y, sm.z) };
//       if (sm.x*sm.y*sm.z != 0) { // child exists
//         I->emplace_back(sb);
//       }
//     }
//     m = p;
//   }
//   I->emplace_back(Set{ 0, xyz2i<int64_t>(n, m.x, m.y, m.z) });
//   std::reverse(I->begin(), I->end());
// }

} // namespace mg

#undef TypeChooser
