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
#define mg_P(N, S, X) ((X) < (N) ? (X) : (X) - (S))

/*
Forward lifting
Flag == 00 if y and z have both not been transformed
Flag == 01 if y has been transformed but not z
Flag == 10 if z has been transformed but not y
Flag == 11 if y and z have both been transformed
*/
#define mg_FLiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
FLiftCdf53##x(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap) {\
  mg_Assert(Flag < 4);\
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);\
  if (D.x == 1) return;\
  mg_Assert(M.x <= N.x);\
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));\
  mg_Assert(D.x >= 3); /* TODO: what if D.x == 2? */\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 2) < M.x);\
  bool IsLastBlock = P.x + D.x * S.x >= M.x;\
  mg_Assert(IsLastBlock || IsOdd(D.x));\
  buffer_t<t> F(Grid->Base.Buffer);\
  int Sx = S.x / 2, Sz = S.z / (1 << (Flag & 1u)), Sy = S.y / (1 << (Flag >> 1));\
  int x1 = mg_P(M.x, Sx, P.x + S.x * (D.x - 1)); /* last position */\
  int x2 = P.x + S.x * (D.x - 2); /* second last position */\
  int x3 = P.x + S.x * (D.x - 3); /* third last position */\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
    int zz = mg_P(M.z, Sz, z);\
    for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
      int yy = mg_P(M.y, Sy, y);\
      if (IsLastBlock) {\
        /* extrapolate */\
        bool Ext = IsEven(D.x);\
        if (Ext) {\
          t A = F[mg_Row##x(x2, yy, zz, N)]; /* 2nd last (even) */\
          t B = F[mg_Row##x(x1, yy, zz, N)]; /* last (odd) */\
          /* store the extrapolated value at the last odd position */\
          F[mg_Row##x(x1, yy, zz, N)] = 2 * B - A;\
        }\
        /* predict (excluding last odd position) */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t & Val = F[mg_Row##x(x, yy, zz, N)];\
          Val -= F[mg_Row##x(x - S.x, yy, zz, N)] / 2;\
          Val -= F[mg_Row##x(x + S.x, yy, zz, N)] / 2;\
        }\
        if (!Ext) { /* no extrapolation, predict at the last odd position */\
          t & Val = F[mg_Row##x(x2, yy, zz, N)];\
          Val -= F[mg_Row##x(x1, yy, zz, N)] / 2;\
          Val -= F[mg_Row##x(x3, yy, zz, N)] / 2;\
        }\
        /* update (excluding last odd position) */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t Val = F[mg_Row##x(x, yy, zz, N)];\
          F[mg_Row##x(x - S.x, yy, zz, N)] += Val / 4;\
          F[mg_Row##x(x + S.x, yy, zz, N)] += Val / 4;\
        }\
        if (!Ext) { /* no extrapolation, update at the last odd position */\
          t Val = F[mg_Row##x(x2, yy, zz, N)];\
          F[mg_Row##x(x1, yy, zz, N)] += Val / 4;\
          F[mg_Row##x(x3, yy, zz, N)] += Val / 4;\
        }\
      } else { /* not last block */\
        /* predict (excluding the last odd position) */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t & Val = F[mg_Row##x(x, yy, zz, N)];\
          Val -= F[mg_Row##x(x - S.x, yy, zz, N)] / 2;\
          Val -= F[mg_Row##x(x + S.x, yy, zz, N)] / 2;\
        }\
        /* predict at the last odd position */\
        t OVal = F[mg_Row##x(x2, yy, zz, N)];\
        OVal  -= F[mg_Row##x(x1, yy, zz, N)] / 2;\
        OVal  -= F[mg_Row##x(x3, yy, zz, N)] / 2;\
        /* update (excluding the last odd position */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t Val = F[mg_Row##x(x, yy, zz, N)];\
          F[mg_Row##x(x - S.x, yy, zz, N)] += Val / 4;\
          F[mg_Row##x(x + S.x, yy, zz, N)] += Val / 4;\
        }\
        /* update at the last odd position */\
        F[mg_Row##x(x3, yy, zz, N)] += OVal / 4; /* update the 2nd last even */\
        if (!Overlap) {\
          F[mg_Row##x(x1, yy, zz, N)] += OVal / 4; /* update the last even */\
          F[mg_Row##x(x2, yy, zz, N)] = OVal; /* write the last odd */\
        }\
      }\
    }\
  }\
}\
} // namespace mg

mg_FLiftCdf53(Z, Y, X) // X forward lifting
mg_FLiftCdf53(Z, X, Y) // Y forward lifting
mg_FLiftCdf53(Y, X, Z) // Z forward lifting
#undef mg_FLiftCdf53

#define mg_ILiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
ILiftCdf53##x(grid_volume* Grid, const v3i& M, u8 Flag, bool Overlap) {\
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);\
  if (D.x == 1) return;\
  mg_Assert(M.x <= N.x);\
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));\
  mg_Assert(D.x >= 3);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 2) < M.x);\
  bool IsLastBlock = P.x + D.x * S.x >= M.x;\
  mg_Assert(IsLastBlock || IsOdd(D.x));\
  buffer_t<t> F(Grid->Base.Buffer);\
  int Sx = S.x / 2, Sy = S.y / (1 << (Flag >> 1)), Sz = S.z / (1 << (Flag & 1u));\
  int x1 = mg_P(M.x, Sx, P.x + S.x * (D.x - 1)); /* last position */\
  int x2 = P.x + S.x * (D.x - 2); /* second last position */\
  int x3 = P.x + S.x * (D.x - 3); /* third last position */\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
    int zz = mg_P(M.z, Sz, z);\
    for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
      int yy = mg_P(M.y, Sy, y);\
      if (IsLastBlock) {\
        printf("last block\n");\
        /* inverse update (excluding last odd position) */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t Val = F[mg_Row##x(x, yy, zz, N)];\
          F[mg_Row##x(x - S.x, yy, zz, N)] -= Val / 4;\
          F[mg_Row##x(x + S.x, yy, zz, N)] -= Val / 4;\
        }\
        bool Ext = IsEven(D.x);\
        if (!Ext) { /* no extrapolation, inverse update at the last odd position */\
          t Val = F[mg_Row##x(x2, yy, zz, N)];\
          F[mg_Row##x(x1, yy, zz, N)] -= Val / 4;\
          F[mg_Row##x(x3, yy, zz, N)] -= Val / 4;\
        } else { /* extrapolation, need to "fix" the last position (odd) */\
          t& A = F[mg_Row##x(x1, yy, zz, N)];\
          t  B = F[mg_Row##x(x2, yy, zz, N)];\
          A = A /2 + B / 2;\
        }\
        for (int I = 0; I < 10; ++I)\
          printf("%.2f ",F[I]);\
        printf("\n");\
        /* inverse predict (excluding last odd position) */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t & Val = F[mg_Row##x(x, yy, zz, N)];\
          Val += F[mg_Row##x(x - S.x, yy, zz, N)] / 2;\
          Val += F[mg_Row##x(x + S.x, yy, zz, N)] / 2;\
        }\
        if (!Ext) { /* no extrapolation, inverse predict at the last odd position */\
          t & Val = F[mg_Row##x(x2, yy, zz, N)];\
          Val += F[mg_Row##x(x1, yy, zz, N)] / 2;\
          Val += F[mg_Row##x(x3, yy, zz, N)] / 2;\
        }\
      } else { /* not the last block */\
        /* inverse update (excluding the last odd position */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t Val = F[mg_Row##x(x, yy, zz, N)];\
          F[mg_Row##x(x - S.x, yy, zz, N)] -= Val / 4;\
          F[mg_Row##x(x + S.x, yy, zz, N)] -= Val / 4;\
        }\
        /* inverse update at the last odd position */\
        t& OVal = F[mg_Row##x(x2, yy, zz, N)];\
        F[mg_Row##x(x3, yy, zz, N)] -= OVal / 4; /* inverse update the 2nd last even */\
        if (!Overlap) {\
          F[mg_Row##x(x1, yy, zz, N)] -= OVal / 4; /* inverse update the last even */\
          /* inverse predict the last odd position */\
          OVal += F[mg_Row##x(x1, yy, zz, N)] / 2;\
          OVal += F[mg_Row##x(x3, yy, zz, N)] / 2;\
        }\
        /* inverse predict (excluding the last odd position) */\
        for (int x = P.x + S.x; x < P.x + S.x * (D.x - 2); x += 2 * S.x) {\
          t & Val = F[mg_Row##x(x, yy, zz, N)];\
          Val += F[mg_Row##x(x - S.x, yy, zz, N)] / 2;\
          Val += F[mg_Row##x(x + S.x, yy, zz, N)] / 2;\
        }\
      }\
    }\
  }\
}\
} // namespace mg

mg_ILiftCdf53(Z, Y, X) // X inverse lifting
mg_ILiftCdf53(Z, X, Y) // Y inverse lifting
mg_ILiftCdf53(Y, X, Z) // Z inverse lifting
#undef mg_ILiftCdf53

// TODO: rethink the volume, extent abstraction, since for the wavelet function
// to work, we will need another dims

// TODO: rework the code that computes the size of the padded region

#undef mg_P

/* Forward x lifting */
// TODO: merge the first two loops
#define mg_FLiftCdf53Old(z, y, x)\
namespace mg {\
mg_T(t) void \
FLiftCdf53Old##x(t* F, const v3i& N, const v3i& L) {\
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

mg_FLiftCdf53Old(Z, Y, X) // X forward lifting
mg_FLiftCdf53Old(Z, X, Y) // Y forward lifting
mg_FLiftCdf53Old(Y, X, Z) // Z forward lifting
#undef mg_LiftCdf53

// TODO: merge two loops
#define mg_ILiftCdf53Old(z, y, x)\
namespace mg {\
mg_T(t) void \
ILiftCdf53Old##x(t* F, const v3i& N, const v3i& L) {\
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

mg_ILiftCdf53Old(Z, Y, X) // X inverse lifting
mg_ILiftCdf53Old(Z, X, Y) // Y inverse lifting
mg_ILiftCdf53Old(Y, X, Z) // Z inverse lifting
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
  return ILiftCdf53Old##x(F, NBig, L);\
}\
} // namespace mg

mg_ILiftExtCdf53(Z, Y, X) // X forward lifting
mg_ILiftExtCdf53(Z, X, Y) // Y forward lifting
mg_ILiftExtCdf53(Y, X, Z) // Z forward lifting
#undef mg_ILiftExtCdf53

#undef mg_RowX
#undef mg_RowY
#undef mg_RowZ

