#pragma once

#include "mg_assert.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_types.h"

#define mg_IdxX(x, y, z, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)
#define mg_IdxY(y, x, z, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)
#define mg_IdxZ(z, x, y, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)

/* Forward x lifting */
#define mg_ForwardLiftCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void ForwardLiftCdf53##x(t* F, v3i N, v3i L) {\
  v3i P(1 << L.X, 1 << L.Y, 1 << L.Z);\
  v3i M = (N + P - 1) / P;\
  if (M.x <= 1)\
    return;\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    t FLeft  =               F[mg_Idx##x(x - 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_Idx##x(x + 1, y, z, N)] : FLeft;\
    F[mg_Idx##x(x, y, z, N)] -= (FLeft + FRight) / 2;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 0; x < M.x; x += 2) {\
    t FLeft  = x > 0       ? F[mg_Idx##x(x - 1, y, z, N)] : F[mg_Idx##x(x + 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_Idx##x(x + 1, y, z, N)] : FLeft;\
    F[mg_Idx##x(x, y, z, N)] += (FLeft + FRight) / 4;\
  }}}\
  mg_HeapArray(Temp, t, M.x / 2);\
  int S##x = (M.x + 1) / 2;\
  for (int z = 0; z < M.z; ++z) {\
  for (int y = 0; y < M.y; ++y) {\
    for (int x = 1; x < M.x; x += 2) {\
      Temp[x / 2]                       = F[mg_Idx##x(x    , y, z, N)];\
      F[mg_Idx##x(x / 2, y, z, N)] = F[mg_Idx##x(x - 1, y, z, N)];\
    }\
    if (IsOdd(M.x))\
      F[mg_Idx##x(M.x / 2, y, z, N)] = F[mg_Idx##x(M.x - 1, y, z, N)];\
    for (int x = 0; x < (M.x / 2); ++x)\
      F[mg_Idx##x(S##x + x, y, z, N)] = Temp[x];\
  }}\
}\
} // namespace mg

mg_ForwardLiftCdf53(Z, Y, X) // X forward lifting
mg_ForwardLiftCdf53(Z, X, Y) // Y forward lifting
mg_ForwardLiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_ForwardLiftCdf53

#define mg_InverseLiftCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void InverseLiftCdf53##x(t* F, v3i N, v3i L) {\
  v3i P(1 << L.X, 1 << L.Y, 1 << L.Z);\
  v3i M = (N + P - 1) / P;\
  if (M.x <= 1)\
    return;\
  mg_HeapArray(Temp, t, M.x / 2);\
  int S##x = (M.x + 1) >> 1;\
  for (int z = 0; z < M.z; ++z) {\
  for (int y = 0; y < M.y; ++y) {\
    for (int x = 0; x < (M.x / 2); ++x)\
      Temp[x] = F[mg_Idx##x(S##x + x, y, z, N)];\
    if (IsOdd(M.x))\
      F[mg_Idx##x(M.x - 1, y, z, N)] = F[mg_Idx##x(M.x >> 1, y, z, N)];\
    for (int x = (M.x / 2) * 2 - 1; x >= 1; x -= 2) {\
      F[mg_Idx##x(x - 1, y, z, N)] = F[mg_Idx##x(x >> 1, y, z, N)];\
      F[mg_Idx##x(x    , y, z, N)] = Temp[x / 2];\
    }\
  }}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 0; x < M.x; x += 2) {\
    t FLeft  = x > 0       ? F[mg_Idx##x(x - 1, y, z, N)] : F[mg_Idx##x(x + 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_Idx##x(x + 1, y, z, N)] : FLeft;\
    F[mg_Idx##x(x, y, z, N)] -= (FLeft + FRight) / 4;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    t FLeft  =               F[mg_Idx##x(x - 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_Idx##x(x + 1, y, z, N)] : FLeft;\
    F[mg_Idx##x(x, y, z, N)] += (FLeft + FRight) / 2;\
  }}}\
}\
} // namespace mg

mg_InverseLiftCdf53(Z, Y, X) // X inverse lifting
mg_InverseLiftCdf53(Z, X, Y) // Y inverse lifting
mg_InverseLiftCdf53(Y, X, Z) // Z inverse lifting
#undef mg_InverseLiftCdf53

#undef mg_IdxX
#undef mg_IdxX
#undef mg_IdxX

namespace mg {
inline array<v3i, 2> BoundLevel(v3i N, int L) {
  for (int I = 0; I < L; ++I) {
    N.X = ((N.X >> 1) << 1) + 1; N.Y = ((N.Y >> 1) << 1) + 1; N.Z = ((N.Z >> 1) << 1) + 1;
    N.X = (N.X + 1) >> 1; N.Y = (N.Y + 1) >> 1; N.Z = (N.Z + 1) >> 1;
  }
  v3i M = N;
  N.X = ((N.X >> 1) << 1) + 1; N.Y = ((N.Y >> 1) << 1) + 1; N.Z = ((N.Z >> 1) << 1) + 1;
  return array<v3i, 2>{ M, N };
}
} // namespace mg

/*  */
#define mg_ForwardLiftExtrapolateCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void ForwardLiftExtrapolateCdf53##x(t* F, v3i N, v3i L) {\
  auto Bl = BoundLevel(N, L);\
  v3i M = Bl[0], P = Bl[1];\
  /* linear extrapolate */\
  if (M.x < P.x) {\
    mg_Assert(M.x + 1 == P.x);\
    for (int z = 0; z < P.z; ++z) {\
    for (int y = 0; y < P.y; ++y) {\
      t A = F[mg_Idx##x(M.x - 2, y, z, N)];\
      t B = F[mg_Idx##x(M.x - 1, y, z, N)];\
      F[mg_Idx##x(M.x, y, z, N)] = 2 * B - A;\
    }}\
  }\
  ForwardLiftCdf53##x(F, N, L);\
}\
} // namespace mg
