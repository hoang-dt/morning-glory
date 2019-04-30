#pragma once

#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_types.h"
#include "mg_volume.h"
#include <stlab/concurrency/future.hpp>

#define mg_IdxX(x, y, z, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)
#define mg_IdxY(y, x, z, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)
#define mg_IdxZ(z, x, y, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)

/* Translate a wavelet coordinate to a storage coordinate */
#define TX(N, X) ((X) <= N.X ? (X) : (N.X + Log2Floor(X - N.X)))
#define TY(N, Y) ((Y) <= N.Y ? (Y) : (N.Y + Log2Floor(Y - N.Y)))
#define TZ(N, Z) ((Z) <= N.Z ? (Z) : (N.Z + Log2Floor(Z - N.Z)))

/* Async forward lifting */
#define mg_FLiftCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void FLiftCdf53##x(const volume& Vol, const extent& Ext) {\
  v3i P = Pos(Ext), D = Dims(Ext), S = Strides(Ext);\
  v3i N = BigDims(Vol), M = SmallDims(Vol);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(IsOdd(D.x));\
  mg_Assert(P.x + S.x * D.x <= N.x);\
  typed_buffer<t> F(Vol.Buffer);\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
  for (int x = P.x + S.x; x < P.x + S.x * D.x; x += 2 * S.x) {\
    printf("%d %d %d %d\n", T##x(M, x), T##x(M, x + S.x), T##y(M, y), T##z(M, z));\
    int yy = T##y(M, y), zz = T##z(M, z);\
    t & Val = F[mg_Idx##x(T##x(M, x), yy, zz, N)];\
    Val -= F[mg_Idx##x(T##x(M, x - S.x), yy, zz, N)] / 2;\
    Val -= F[mg_Idx##x(T##x(M, x + S.x), yy, zz, N)] / 2;\
  }}}\
  printf("hello\n");\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
  for (int x = P.x + S.x; x < P.x + S.x * D.x; x += 2 * S.x) {\
    printf("%d %d %d %d\n", T##x(M, x), T##x(M, x + S.x), T##y(M, y), T##z(M, z));\
    int yy = T##y(M, y), zz = T##z(M, z);\
    t Val = F[mg_Idx##x(T##x(M, x), yy, zz, N)];\
    F[mg_Idx##x(T##x(M, x - S.x), yy, zz, N)] += Val / 4;\
    F[mg_Idx##x(T##x(M, x + S.x), yy, zz, N)] += Val / 4;\
  }}}\
  printf("hello2\n");\
}\
} // namespace mg

mg_FLiftCdf53(Z, Y, X) // X forward lifting
mg_FLiftCdf53(Z, X, Y) // Y forward lifting
mg_FLiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_FLiftCdf53

#define mg_ILiftCdf53(z, y, x)\
namespace mg {\
template <typename t>\
stlab::future<void> ILiftCdf53##x(const volume& Vol, const extent& Ext) {\
  v3i P = Pos(Ext), D = Dims(Ext), S = Strides(Ext);\
  v3i N = BigDims(Vol), M = SmallDims(Vol);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(IsOdd(D.x));\
  mg_Assert(P.x + S.x * D.x <= N.x);\
  typed_buffer<t> F(Vol.Buffer);\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
  for (int x = P.x; x < P.x + S.x * D.x; x += S.x * 2) {\
    int yy = T##y(M, y), zz = T##z(M, z);\
    t Val = F[mg_Idx##x(T##x(M, x), yy, zz, N)];\
    F[mg_Idx##x(T##x(M, x - 1), yy, zz, N)] -= Val / 4;\
    F[mg_Idx##x(T##x(M, x + 1), yy, zz, N)] -= Val / 4;\
  }}}\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
  for (int x = P.x; x < P.x + S.x * D.x; x += S.x * 2) {\
    int yy = T##y(M, y), zz = T##z(M, z);\
    t & Val = F[mg_Idx##x(T##x(M, x), yy, zz, N)];\
    Val += F[mg_Idx##x(T##x(M, x - 1), yy, zz, N)] / 2;\
    Val += F[mg_Idx##x(T##x(M, x + 1), yy, zz, N)] / 2;\
  }}}\
}\
} // namespace mg

mg_ILiftCdf53(Z, Y, X) // X forward lifting
mg_ILiftCdf53(Z, X, Y) // Y forward lifting
mg_ILiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_ILiftCdf53

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
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t & Val = F[mg_Idx##x(x, y, z, N)];\
    Val -= F[mg_Idx##x(XLeft, y, z, N)] / 2;\
    Val -= F[mg_Idx##x(XRight, y, z, N)] / 2;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t Val = F[mg_Idx##x(x, y, z, N)];\
    F[mg_Idx##x(XLeft, y, z, N)] += Val / 4;\
    F[mg_Idx##x(XRight, y, z, N)] += Val / 4;\
  }}}\
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
  for (int x = 1; x < M.x; x += 2) {\
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t Val = F[mg_Idx##x(x, y, z, N)];\
    F[mg_Idx##x(XLeft, y, z, N)] -= Val / 4;\
    F[mg_Idx##x(XRight, y, z, N)] -= Val / 4;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t & Val = F[mg_Idx##x(x, y, z, N)];\
    Val += F[mg_Idx##x(XLeft, y, z, N)] / 2;\
    Val += F[mg_Idx##x(XRight, y, z, N)] / 2;\
  }}}\
}\
} // namespace mg

mg_InverseLiftCdf53(Z, Y, X) // X inverse lifting
mg_InverseLiftCdf53(Z, X, Y) // Y inverse lifting
mg_InverseLiftCdf53(Y, X, Z) // Z inverse lifting
#undef mg_InverseLiftCdf53

namespace mg {
inline array<v3i, 2> DimsAtLevel(v3i N, int L) {
  for (int I = 0; I < L; ++I) {
    N = ((N / 2) * 2) + 1;
    N = (N + 1) / 2;
  }
  return array<v3i,2>{N, (N / 2) * 2 + 1};
}
} // namespace mg

#define mg_ForwardLiftExtrapolateCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void ForwardLiftExtrapolateCdf53##x(t* F, v3i N, v3i NBig, v3i L) {\
  mg_Assert(L.X == L.Y && L.Y == L.Z);\
  auto D = DimsAtLevel(N, L.x);\
  /* linearly extrapolate */\
  if (D[0].x < D[1].x) {\
    mg_Assert(D[0].x + 1 == D[1].x);\
    _Pragma("omp parallel for")\
    for (int z = 0; z < D[1].z; ++z) {\
    for (int y = 0; y < D[1].y; ++y) {\
      t A = F[mg_Idx##x(D[0].x - 2, y, z, NBig)];\
      t B = F[mg_Idx##x(D[0].x - 1, y, z, NBig)];\
      F[mg_Idx##x(D[0].x, y, z, NBig)] = 2 * B - A;\
    }}\
  }\
  v3i P(1 << L.X, 1 << L.Y, 1 << L.Z);\
  v3i M = (NBig + P - 1) / P;\
  if (M.x <= 1)\
    return;\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < D[1].x; x += 2) {\
    t & Val = F[mg_Idx##x(x, y, z, NBig)];\
    Val -= F[mg_Idx##x(x - 1, y, z, NBig)] / 2;\
    Val -= F[mg_Idx##x(x + 1, y, z, NBig)] / 2;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < D[1].x; x += 2) {\
    t Val = F[mg_Idx##x(x, y, z, NBig)];\
    F[mg_Idx##x(x - 1, y, z, NBig)] += Val / 4;\
    F[mg_Idx##x(x + 1, y, z, NBig)] += Val / 4;\
  }}}\
  mg_HeapArray(Temp, t, M.x / 2);\
  int S##x = (M.x + 1) / 2;\
  for (int z = 0; z < M.z; ++z) {\
  for (int y = 0; y < M.y; ++y) {\
    for (int x = 1; x < M.x; x += 2) {\
      Temp[x / 2] = F[mg_Idx##x(x, y, z, NBig)];\
      F[mg_Idx##x(x / 2, y, z, NBig)] = F[mg_Idx##x(x - 1, y, z, NBig)];\
    }\
    if (IsOdd(M.x))\
      F[mg_Idx##x(M.x / 2, y, z, NBig)] = F[mg_Idx##x(M.x - 1, y, z, NBig)];\
    for (int x = 0; x < (M.x / 2); ++x)\
      F[mg_Idx##x(S##x + x, y, z, NBig)] = Temp[x];\
  }}\
}\
} // namespace mg

mg_ForwardLiftExtrapolateCdf53(Z, Y, X) // X forward lifting
mg_ForwardLiftExtrapolateCdf53(Z, X, Y) // Y forward lifting
mg_ForwardLiftExtrapolateCdf53(Y, X, Z) // Z forward lifting
#undef mg_ForwardLiftExtrapolateCdf53

#define mg_InverseLiftExtrapolateCdf53(z, y, x)\
namespace mg {\
template <typename t>\
void InverseLiftExtrapolateCdf53##x(t* F, v3i N, v3i NBig, v3i L) {\
  (void)N;\
  mg_Assert(L.X == L.Y && L.Y == L.Z);\
  return InverseLiftCdf53##x(F, NBig, L);\
}\
} // namespace mg

mg_InverseLiftExtrapolateCdf53(Z, Y, X) // X forward lifting
mg_InverseLiftExtrapolateCdf53(Z, X, Y) // Y forward lifting
mg_InverseLiftExtrapolateCdf53(Y, X, Z) // Z forward lifting
#undef mg_InverseLiftExtrapolateCdf53

#undef mg_IdxX
#undef mg_IdxX
#undef mg_IdxX

