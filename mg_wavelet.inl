#pragma once

#include "mg_math.h"
#include "mg_memory.h"
#include "mg_types.h"

#define mg_RowMajorX(x, y, z, N) (z) * N.X * N.Y + (y) * N.X + (x)
#define mg_RowMajorY(y, x, z, N) (z) * N.X * N.Y + (y) * N.X + (x)
#define mg_RowMajorZ(z, x, y, N) (z) * N.X * N.Y + (y) * N.X + (x)

/* Forward x lifting */
#define mg_ForwardLiftCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void ForwardLiftCdf53##x(t* F, v3l N, v3l L) {\
  v3l P(1 << L.X, 1 << L.Y, 1 << L.Z);\
  v3l M((N.X + P.X - 1) / P.X, (N.Y + P.Y - 1) / P.Y, (N.Z + P.Z - 1) / P.Z);\
  if (M.x <= 1)\
    return;\
  _Pragma("omp parallel for collapse(2)")\
  for (i64 z = 0; z < M.z; ++z   ) {\
  for (i64 y = 0; y < M.y; ++y   ) {\
  for (i64 x = 1; x < M.x; x += 2) {\
    t FLeft  =               F[mg_RowMajor##x(x - 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_RowMajor##x(x + 1, y, z, N)] : FLeft;\
    F[mg_RowMajor##x(x, y, z, N)] -= (FLeft + FRight) / 2;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (i64 z = 0; z < M.z; ++z   ) {\
  for (i64 y = 0; y < M.y; ++y   ) {\
  for (i64 x = 0; x < M.x; x += 2) {\
    t FLeft  = x > 0       ? F[mg_RowMajor##x(x - 1, y, z, N)] : F[mg_RowMajor##x(x + 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_RowMajor##x(x + 1, y, z, N)] : FLeft;\
    F[mg_RowMajor##x(x, y, z, N)] += (FLeft + FRight) / 4;\
  }}}\
  t* Temp = nullptr;\
  mg_Allocate(Temp, M.x / 2);\
  i64 S##x = (M.x + 1) / 2;\
  for (i64 z = 0; z < M.z; ++z) {\
  for (i64 y = 0; y < M.y; ++y) {\
    for (i64 x = 1; x < M.x; x += 2) {\
      Temp[x / 2]                       = F[mg_RowMajor##x(x    , y, z, N)];\
      F[mg_RowMajor##x(x / 2, y, z, N)] = F[mg_RowMajor##x(x - 1, y, z, N)];\
    }\
    if (IsOdd(M.x))\
      F[mg_RowMajor##x(M.x / 2, y, z, N)] = F[mg_RowMajor##x(M.x - 1, y, z, N)];\
    for (i64 x = 0; x < (M.x / 2); ++x)\
      F[mg_RowMajor##x(S##x + x, y, z, N)] = Temp[x];\
  }}\
  mg_Deallocate(Temp);\
}\
} // namespace mg

mg_ForwardLiftCdf53(Z, Y, X) // X forward lifting
mg_ForwardLiftCdf53(Z, X, Y) // Y forward lifting
mg_ForwardLiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_FLiftCdf53

#define mg_InverseLiftCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void InverseLiftCdf53##x(t* F, v3l N, v3l L) {\
  v3l P(1 << L.X, 1 << L.Y, 1 << L.Z);\
  v3l M((N.X + P.X - 1) / P.X, (N.Y + P.Y - 1) / P.Y, (N.Z + P.Z - 1) / P.Z);\
  if (M.x <= 1)\
    return;\
  t* Temp = nullptr;\
  mg_Allocate(Temp, M.x / 2);\
  i64 S##x = (M.x + 1) >> 1;\
  for (i64 z = 0; z < M.z; ++z) {\
  for (i64 y = 0; y < M.y; ++y) {\
    for (i64 x = 0; x < (M.x / 2); ++x)\
      Temp[x] = F[mg_RowMajor##x(S##x + x, y, z, N)];\
    if (IsOdd(M.x))\
      F[mg_RowMajor##x(M.x - 1, y, z, N)] = F[mg_RowMajor##x(M.x >> 1, y, z, N)];\
    for (i64 x = (M.x / 2) * 2 - 1; x >= 1; x -= 2) {\
      F[mg_RowMajor##x(x - 1, y, z, N)] = F[mg_RowMajor##x(x>>1, y, z, N)];\
      F[mg_RowMajor##x(x    , y, z, N)] = Temp[x / 2];\
    }\
  }}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 0; x < M.x; x += 2) {\
    t FLeft  = x > 0       ? F[mg_RowMajor##x(x - 1, y, z, N)] : F[mg_RowMajor##x(x + 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_RowMajor##x(x + 1, y, z, N)] : FLeft;\
    F[mg_RowMajor##x(x, y, z, N)] -= (FLeft + FRight) / 4;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    t FLeft  =               F[mg_RowMajor##x(x - 1, y, z, N)];\
    t FRight = x < M.x - 1 ? F[mg_RowMajor##x(x + 1, y, z, N)] : FLeft;\
    F[mg_RowMajor##x(x, y, z, N)] += (FLeft + FRight) / 2;\
  }}}\
  mg_Deallocate(Temp);\
}\
} // namespace mg

mg_InverseLiftCdf53(Z, Y, X) // X inverse lifting
mg_InverseLiftCdf53(Z, X, Y) // Y inverse lifting
mg_InverseLiftCdf53(Y, X, Z) // Z inverse lifting
#undef mg_ILiftCdf53
