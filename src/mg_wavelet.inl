#pragma once

#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_math.h"
#include "mg_memory.h"
//#include <stlab/concurrency/future.hpp>

#define mg_RowX(x, y, z, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)
#define mg_RowY(y, x, z, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)
#define mg_RowZ(z, x, y, N) i64(z) * N.X * N.Y + i64(y) * N.X + (x)

/* Translate a wavelet coordinate to a storage coordinate */
// TODO: precompute the Log2Floor
#define mg_TX(N, S, x) ((x) <= N.X ? (x) : (N.X + Log2Floor(S.X)))
#define mg_TY(N, S, y) ((y) <= N.Y ? (y) : (N.Y + Log2Floor(S.Y)))
#define mg_TZ(N, S, z) ((z) <= N.Z ? (z) : (N.Z + Log2Floor(S.Z)))

/* Async forward lifting */
#define mg_FLiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
FLiftCdf53##x(volume* Vol, grid& Ext) {\
  v3i P = Pos(Ext), D = Dims(Ext), S = Strides(Ext);\
  v3i N = BigDims(Vol), M = SmallDims(Vol);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 1) < N.x);\
  buffer_t<t> F(Vol.Buffer);\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
    int yy = T##y(M, S, y), zz = T##z(M, S, z);\
    /* extrapolate */\
    if (IsEven(D.x)) {\
      t A = F[mg_Row##x(T##x(M, S, P.x + S.x * (D.x - 2)), yy, zz, N)];\
      t B = F[mg_Row##x(T##x(M, S, P.x + S.x * (D.x - 1)), yy, zz, N)];\
      F[mg_Row##x(T##x(M, S, P.x + S.x * D.x), yy, zz, N)] = 2 * B - A;\
    }\
    for (int x = P.x + S.x; x < P.x + S.x * D.x; x += 2 * S.x) {\
      t & Val = F[mg_Row##x(T##x(M, S, x), yy, zz, N)];\
      Val -= F[mg_Row##x(T##x(M, S, x - S.x), yy, zz, N)] / 2;\
      Val -= F[mg_Row##x(T##x(M, S, x + S.x), yy, zz, N)] / 2;\
    }\
    for (int x = P.x + S.x; x < P.x + S.x * D.x; x += 2 * S.x) {\
      t Val = F[mg_Row##x(T##x(M, S, x), yy, zz, N)];\
      F[mg_Row##x(T##x(M, S, x - S.x), yy, zz, N)] += Val / 4;\
      F[mg_Row##x(T##x(M, S, x + S.x), yy, zz, N)] += Val / 4;\
    }\
  }}\
}\
} // namespace mg

//mg_FLiftCdf53(Z, Y, X) // X forward lifting
//mg_FLiftCdf53(Z, X, Y) // Y forward lifting
//mg_FLiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_FLiftCdf53

#define mg_ILiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void\
ILiftCdf53##x(volume& Vol, grid& Ext) {\
  v3i P = Pos(Ext), D = Dims(Ext), S = Strides(Ext);\
  v3i N = BigDims(Vol), M = SmallDims(Vol);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 1) < NextPow2(N.x) + 1);\
  buffer_t<t> F(Vol.Buffer);\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
    int yy = T##y(M, S, y), zz = T##z(M, S, z);\
    for (int x = P.x + S.x; x < P.x + S.x * D.x; x += S.x * 2) {\
      t Val = F[mg_Row##x(T##x(M, S, x), yy, zz, N)];\
      F[mg_Row##x(T##x(M, S, x - S.x), yy, zz, N)] -= Val / 4;\
      F[mg_Row##x(T##x(M, S, x + S.x), yy, zz, N)] -= Val / 4;\
    }\
    for (int x = P.x + S.x; x < P.x + S.x * D.x; x += S.x * 2) {\
      t & Val = F[mg_Row##x(T##x(M, S, x), yy, zz, N)];\
      Val += F[mg_Row##x(T##x(M, S, x - S.x), yy, zz, N)] / 2;\
      Val += F[mg_Row##x(T##x(M, S, x + S.x), yy, zz, N)] / 2;\
    }\
  }}\
}\
} // namespace mg

//mg_ILiftCdf53(Z, Y, X) // X inverse lifting
//mg_ILiftCdf53(Z, X, Y) // Y inverse lifting
//mg_ILiftCdf53(Y, X, Z) // Z inverse lifting
#undef mg_ILiftCdf53

// TODO: rethink the volume, extent abstraction, since for the wavelet function
// to work, we will need another dims

// TODO: rework the code that computes the size of the padded region

#define mg_ILiftUnpackCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
ILiftUnpackCdf53##x(volume& Vol, grid& Ext, int Pass) {\
  v3i P = Pos(Ext), D = Dims(Ext), S = Strides(Ext);\
  v3i N = BigDims(Vol), M = SmallDims(Vol);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 1) < NextPow2(N.x) + 1);\
  buffer_t<t> F(Vol.Buffer);\
  int Start = P.x + ((M.x - 1 - P.x) / (2 * S.x) + 1) * (2 * S.x);\
  int End = P.x + ((D.x - 1) / 2) * (2 * S.x);\
  printf("End%d Start%d\n", End, Start);\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
  for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
    /* NOTE: the code below is correct for transform order (X, Y, Z), but */\
    /* for transform order (Z, Y, X), the following must be used */\
    /* int yyy = Pass == 0 ? yy : y, zzz = Pass == 2 ? z : zz; */\
    int yy = Pass == 2 ? y : T##y(M, S, y), zz = Pass == 0 ? T##z(M, S, z) : z;\
    for (int x = End; x >= Start; x -= 2 * S.x) {\
      int I = mg_Row##x(x, yy, zz, N);\
      int J = T##x(M, S, x);\
      printf("zz %d yy %d x %d I %d J %d N %d %d %d\n", z, y, x, I, J, N.X, N.Y, N.Z);\
      F[mg_Row##x(x, yy, zz, N)] = F[mg_Row##x(T##x(M, S, x), yy, zz, N)];\
    }\
  }}\
}\
} // namespace mg

//mg_ILiftUnpackCdf53(Z, Y, X) // X inverse lifting
//mg_ILiftUnpackCdf53(Z, X, Y) // Y inverse lifting
//mg_ILiftUnpackCdf53(Y, X, Z) // Z inverse lifting
#undef mg_ILiftCdf53

#define mg_Unpack(z, y, x)\
namespace mg {\
mg_T(t) void \
Unpack##x(volume& Vol, grid& Ext, int Pass) {\
  v3i P = Pos(Ext), D = Dims(Ext), S = Strides(Ext);\
  v3i N = BigDims(Vol), M = SmallDims(Vol);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(M.x > P.x);\
  mg_Assert(P.x + S.x * (D.x - 1) < NextPow2(N.x) + 1);\
  buffer_t<t> F(Vol.Buffer);\
  int Start = P.x + ((M.x - P.x) / S.x + 1) * S.x;\
  if (P.x + S.x * (D.x - 1) >= M.x) {\
    for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
    for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
      int yy = T##y(M, S, y), zz = T##z(M, S, z);\
      /* NOTE: the code below is correct for transform order (X, Y, Z), but */\
      /* for transform order (Z, Y, X), the following must be used */\
      /* int yyy = Pass == 0 ? yy : y, zzz = Pass == 2 ? z : zz; */\
      int yyy = Pass == 2 ? y : yy, zzz = Pass == 0 ? zz : z;\
      for (int x = P.x + S.x * (D.x - 1); x >= Start; x -= S.x)\
        F[mg_Row##x(x, yyy, zzz, N)] = F[mg_Row##x(T##x(M, S, x), yyy, zzz, N)];\
    }}\
  }\
}\
} // namespace mg
//mg_Unpack(Z, Y, X) // X unpacking
//mg_Unpack(Z, X, Y) // Y unpacking
//mg_Unpack(Y, X, Z) // Z unpacking
#undef mg_Unpack

/* Forward x lifting */
// TODO: merge the first two loops
#define mg_FLiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
FLiftCdf53##x(t* F, const v3i& N, const v3i& L) {\
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
    t & Val = F[mg_Row##x(x, y, z, N)];\
    Val -= F[mg_Row##x(XLeft, y, z, N)] / 2;\
    Val -= F[mg_Row##x(XRight, y, z, N)] / 2;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t Val = F[mg_Row##x(x, y, z, N)];\
    F[mg_Row##x(XLeft, y, z, N)] += Val / 4;\
    F[mg_Row##x(XRight, y, z, N)] += Val / 4;\
  }}}\
  mg_MallocArray(Temp, t, M.x / 2);\
  int S##x = (M.x + 1) / 2;\
  for (int z = 0; z < M.z; ++z) {\
  for (int y = 0; y < M.y; ++y) {\
    for (int x = 1; x < M.x; x += 2) {\
      Temp[x / 2] = F[mg_Row##x(x    , y, z, N)];\
      F[mg_Row##x(x / 2, y, z, N)] = F[mg_Row##x(x - 1, y, z, N)];\
    }\
    if (IsOdd(M.x))\
      F[mg_Row##x(M.x / 2, y, z, N)] = F[mg_Row##x(M.x - 1, y, z, N)];\
    for (int x = 0; x < (M.x / 2); ++x)\
      F[mg_Row##x(S##x + x, y, z, N)] = Temp[x];\
  }}\
}\
} // namespace mg

mg_FLiftCdf53(Z, Y, X) // X forward lifting
mg_FLiftCdf53(Z, X, Y) // Y forward lifting
mg_FLiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_LiftCdf53

// TODO: merge two loops
#define mg_ILiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
ILiftCdf53##x(t* F, const v3i& N, const v3i& L) {\
  v3i P(1 << L.X, 1 << L.Y, 1 << L.Z);\
  v3i M = (N + P - 1) / P;\
  if (M.x <= 1)\
    return;\
  mg_MallocArray(Temp, t, M.x / 2);\
  int S##x = (M.x + 1) >> 1;\
  for (int z = 0; z < M.z; ++z) {\
  for (int y = 0; y < M.y; ++y) {\
    for (int x = 0; x < (M.x / 2); ++x)\
      Temp[x] = F[mg_Row##x(S##x + x, y, z, N)];\
    if (IsOdd(M.x))\
      F[mg_Row##x(M.x - 1, y, z, N)] = F[mg_Row##x(M.x >> 1, y, z, N)];\
    for (int x = (M.x / 2) * 2 - 1; x >= 1; x -= 2) {\
      F[mg_Row##x(x - 1, y, z, N)] = F[mg_Row##x(x >> 1, y, z, N)];\
      F[mg_Row##x(x    , y, z, N)] = Temp[x / 2];\
    }\
  }}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t Val = F[mg_Row##x(x, y, z, N)];\
    F[mg_Row##x(XLeft, y, z, N)] -= Val / 4;\
    F[mg_Row##x(XRight, y, z, N)] -= Val / 4;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < M.x; x += 2) {\
    int XLeft = x - 1;\
    int XRight = x < M.x - 1 ? x + 1 : x - 1;\
    t & Val = F[mg_Row##x(x, y, z, N)];\
    Val += F[mg_Row##x(XLeft, y, z, N)] / 2;\
    Val += F[mg_Row##x(XRight, y, z, N)] / 2;\
  }}}\
}\
} // namespace mg

mg_ILiftCdf53(Z, Y, X) // X inverse lifting
mg_ILiftCdf53(Z, X, Y) // Y inverse lifting
mg_ILiftCdf53(Y, X, Z) // Z inverse lifting
#undef mg_ILiftCdf53

namespace mg {
inline stack_array<v3i, 2> 
DimsAtLevel(v3i N, int L) {
  for (int I = 0; I < L; ++I) {
    N = ((N / 2) * 2) + 1;
    N = (N + 1) / 2;
  }
  return stack_array<v3i,2>{N, (N / 2) * 2 + 1};
}
} // namespace mg

#define mg_FLiftExtCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
FLiftExtCdf53##x(t* F, const v3i& N, const v3i& NBig, const v3i& L) {\
  mg_Assert(L.X == L.Y && L.Y == L.Z);\
  auto D = DimsAtLevel(N, L.x);\
  /* linearly extrapolate */\
  if (D[0].x < D[1].x) {\
    mg_Assert(D[0].x + 1 == D[1].x);\
    _Pragma("omp parallel for")\
    for (int z = 0; z < D[1].z; ++z) {\
    for (int y = 0; y < D[1].y; ++y) {\
      t A = F[mg_Row##x(D[0].x - 2, y, z, NBig)];\
      t B = F[mg_Row##x(D[0].x - 1, y, z, NBig)];\
      F[mg_Row##x(D[0].x, y, z, NBig)] = 2 * B - A;\
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
    t & Val = F[mg_Row##x(x, y, z, NBig)];\
    Val -= F[mg_Row##x(x - 1, y, z, NBig)] / 2;\
    Val -= F[mg_Row##x(x + 1, y, z, NBig)] / 2;\
  }}}\
  _Pragma("omp parallel for collapse(2)")\
  for (int z = 0; z < M.z; ++z   ) {\
  for (int y = 0; y < M.y; ++y   ) {\
  for (int x = 1; x < D[1].x; x += 2) {\
    t Val = F[mg_Row##x(x, y, z, NBig)];\
    F[mg_Row##x(x - 1, y, z, NBig)] += Val / 4;\
    F[mg_Row##x(x + 1, y, z, NBig)] += Val / 4;\
  }}}\
  mg_MallocArray(Temp, t, M.x / 2);\
  int S##x = (M.x + 1) / 2;\
  for (int z = 0; z < M.z; ++z) {\
  for (int y = 0; y < M.y; ++y) {\
    for (int x = 1; x < M.x; x += 2) {\
      Temp[x / 2] = F[mg_Row##x(x, y, z, NBig)];\
      F[mg_Row##x(x / 2, y, z, NBig)] = F[mg_Row##x(x - 1, y, z, NBig)];\
    }\
    if (IsOdd(M.x))\
      F[mg_Row##x(M.x / 2, y, z, NBig)] = F[mg_Row##x(M.x - 1, y, z, NBig)];\
    for (int x = 0; x < (M.x / 2); ++x)\
      F[mg_Row##x(S##x + x, y, z, NBig)] = Temp[x];\
  }}\
}\
} // namespace mg

mg_FLiftExtCdf53(Z, Y, X) // X forward lifting
mg_FLiftExtCdf53(Z, X, Y) // Y forward lifting
mg_FLiftExtCdf53(Y, X, Z) // Z forward lifting
#undef mg_FLiftExtCdf53

#define mg_ILiftExtCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
ILiftExtCdf53##x(t* F, const v3i& N, const v3i& NBig, const v3i& L) {\
  (void)N;\
  mg_Assert(L.X == L.Y && L.Y == L.Z);\
  return ILiftCdf53##x(F, NBig, L);\
}\
} // namespace mg

mg_ILiftExtCdf53(Z, Y, X) // X forward lifting
mg_ILiftExtCdf53(Z, X, Y) // Y forward lifting
mg_ILiftExtCdf53(Y, X, Z) // Z forward lifting
#undef mg_ILiftExtCdf53

#undef mg_RowX
#undef mg_RowY
#undef mg_RowZ

