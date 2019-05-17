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

namespace mg {
mg_T(t) void
FLiftCdf53ZTest(grid_volume* Grid, const v3i& M, bool Overlap) {
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);
  if (D.Z == 1) return;
  mg_Assert(M.Z <= N.Z);
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));
  mg_Assert(D.Z >= 2);
  mg_Assert(IsEven(P.Z));
  mg_Assert(P.Z + S.Z * (D.Z - 2) < M.Z);
  bool IsLastBlock = P.Z + D.Z * S.Z >= M.Z;
  mg_Assert(IsLastBlock || IsOdd(D.Z));
  buffer_t<t> F(Grid->Base.Buffer);
  int x1 = Min(M.Z, P.Z + S.Z * (D.Z - 1));
  int x2 = P.Z + S.Z * (D.Z - 2);
  int x3 = P.Z + S.Z * (D.Z - 3);
  for (int Y = P.Y; Y < P.Y + S.Y * D.Y; Y += S.Y) {
    int zz = Min(Y, M.Y);
    for (int X = P.X; X < P.X + S.X * D.X; X += S.X) {
      int yy = Min(X, M.X);
      if (IsLastBlock) {
        bool Ext = IsEven(D.Z);
        if (Ext) {
          t A = F[mg_RowZ(x2, yy, zz, N)];
          t B = F[mg_RowZ(x1, yy, zz, N)];
          F[mg_RowZ(M.Z, yy, zz, N)] = 2 * B - A;
        }
        for (int Z = P.Z + S.Z; Z < P.Z + S.Z * (D.Z - 2); Z += 2 * S.Z) {
          t & Val = F[mg_RowZ(Z, yy, zz, N)];
          Val -= F[mg_RowZ(Z - S.Z, yy, zz, N)] / 2;
          Val -= F[mg_RowZ(Z + S.Z, yy, zz, N)] / 2;
        }
        if (!Ext) {
          t & Val = F[mg_RowZ(x2, yy, zz, N)];
          Val -= F[mg_RowZ(x1, yy, zz, N)] / 2;
          Val -= F[mg_RowZ(x3, yy, zz, N)] / 2;
        } else {
          F[mg_RowZ(x1, yy, zz, N)] = 0;
        }
        for (int Z = P.Z + S.Z; Z < P.Z + S.Z * (D.Z - 2); Z += 2 * S.Z) {
          t Val = F[mg_RowZ(Z, yy, zz, N)];
          F[mg_RowZ(Z - S.Z, yy, zz, N)] += Val / 4;
          F[mg_RowZ(Z + S.Z, yy, zz, N)] += Val / 4;
        }
        if (!Ext) {
          t Val = F[mg_RowZ(x2, yy, zz, N)];
          F[mg_RowZ(x1, yy, zz, N)] += Val / 4;
          F[mg_RowZ(x3, yy, zz, N)] += Val / 4;
        }
      } else {
        for (int Z = P.Z + S.Z; Z < P.Z + S.Z * (D.Z - 2); Z += 2 * S.Z) {
          t & Val = F[mg_RowZ(Z, yy, zz, N)];
          Val -= F[mg_RowZ(Z - S.Z, yy, zz, N)] / 2;
          Val -= F[mg_RowZ(Z + S.Z, yy, zz, N)] / 2;
        }
        t OVal = F[mg_RowZ(x2, yy, zz, N)];
        OVal -= F[mg_RowZ(x1, yy, zz, N)] / 2;
        OVal -= F[mg_RowZ(x3, yy, zz, N)] / 2;
        for (int Z = P.Z + S.Z; Z < P.Z + S.Z * (D.Z - 2); Z += 2 * S.Z) {
          t Val = F[mg_RowZ(Z, yy, zz, N)];
          F[mg_RowZ(Z - S.Z, yy, zz, N)] += Val / 4;
          F[mg_RowZ(Z + S.Z, yy, zz, N)] += Val / 4;
        }
        F[mg_RowZ(x3, yy, zz, N)] += OVal / 4;
        if (!Overlap) {
          F[mg_RowZ(x1, yy, zz, N)] += OVal / 4;
          F[mg_RowZ(x2, yy, zz, N)] = OVal;
        }
      }
    }
  }
}
}

namespace mg {
mg_T(t) void
FLiftCdf53YTest(grid_volume* Grid, const v3i& M, bool Overlap) {
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);
  if (D.Y == 1) return;
  mg_Assert(M.Y <= N.Y);
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));
  mg_Assert(D.Y >= 2);
  mg_Assert(IsEven(P.Y));
  mg_Assert(P.Y + S.Y * (D.Y - 2) < M.Y);
  bool IsLastBlock = P.Y + D.Y * S.Y >= M.Y;
  mg_Assert(IsLastBlock || IsOdd(D.Y));
  buffer_t<t> F(Grid->Base.Buffer);
  int x1 = Min(M.Y, P.Y + S.Y * (D.Y - 1));
  int x2 = P.Y + S.Y * (D.Y - 2);
  int x3 = P.Y + S.Y * (D.Y - 3);
  for (int Z = P.Z; Z < P.Z + S.Z * D.Z; Z += S.Z) {
    int zz = Min(Z, M.Z);
    for (int X = P.X; X < P.X + S.X * D.X; X += S.X) {
      int yy = Min(X, M.X);
      if (IsLastBlock) {
        bool Ext = IsEven(D.Y);
        if (Ext) {
          t A = F[mg_RowY(x2, yy, zz, N)];
          t B = F[mg_RowY(x1, yy, zz, N)];
          F[mg_RowY(M.Y, yy, zz, N)] = 2 * B - A;
        }
        for (int Y = P.Y + S.Y; Y < P.Y + S.Y * (D.Y - 2); Y += 2 * S.Y) {
          t & Val = F[mg_RowY(Y, yy, zz, N)];
          Val -= F[mg_RowY(Y - S.Y, yy, zz, N)] / 2;
          Val -= F[mg_RowY(Y + S.Y, yy, zz, N)] / 2;
        }
        if (!Ext) {
          t & Val = F[mg_RowY(x2, yy, zz, N)];
          Val -= F[mg_RowY(x1, yy, zz, N)] / 2;
          Val -= F[mg_RowY(x3, yy, zz, N)] / 2;
        } else {
          F[mg_RowY(x1, yy, zz, N)] = 0;
        }
        for (int Y = P.Y + S.Y; Y < P.Y + S.Y * (D.Y - 2); Y += 2 * S.Y) {
          t Val = F[mg_RowY(Y, yy, zz, N)];
          F[mg_RowY(Y - S.Y, yy, zz, N)] += Val / 4;
          F[mg_RowY(Y + S.Y, yy, zz, N)] += Val / 4;
        }
        if (!Ext) {
          t Val = F[mg_RowY(x2, yy, zz, N)];
          F[mg_RowY(x1, yy, zz, N)] += Val / 4;
          F[mg_RowY(x3, yy, zz, N)] += Val / 4;
        }
      } else {
        for (int Y = P.Y + S.Y; Y < P.Y + S.Y * (D.Y - 2); Y += 2 * S.Y) {
          t & Val = F[mg_RowY(Y, yy, zz, N)];
          Val -= F[mg_RowY(Y - S.Y, yy, zz, N)] / 2;
          Val -= F[mg_RowY(Y + S.Y, yy, zz, N)] / 2;
        }
        t OVal = F[mg_RowY(x2, yy, zz, N)];
        OVal -= F[mg_RowY(x1, yy, zz, N)] / 2;
        OVal -= F[mg_RowY(x3, yy, zz, N)] / 2;
        for (int Y = P.Y + S.Y; Y < P.Y + S.Y * (D.Y - 2); Y += 2 * S.Y) {
          t Val = F[mg_RowY(Y, yy, zz, N)];
          F[mg_RowY(Y - S.Y, yy, zz, N)] += Val / 4;
          F[mg_RowY(Y + S.Y, yy, zz, N)] += Val / 4;
        }
        F[mg_RowY(x3, yy, zz, N)] += OVal / 4;
        if (!Overlap) {
          F[mg_RowY(x1, yy, zz, N)] += OVal / 4;
          F[mg_RowY(x2, yy, zz, N)] = OVal;
        }
      }
    }
  }
}
}

namespace mg {
mg_T(t) void
FLiftCdf53XTest(grid_volume* Grid, const v3i& M, bool Overlap) {
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);
  if (D.X == 1) return;
  mg_Assert(M.X <= N.X);
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));
  mg_Assert(D.X >= 2); /* TODO: what if D.x == 2? */
  mg_Assert(IsEven(P.X));
  mg_Assert(P.X + S.X * (D.X - 2) < M.X);
  bool IsLastBlock = P.X + D.X * S.X >= M.X;
  mg_Assert(IsLastBlock || IsOdd(D.X));
  buffer_t<t> F(Grid->Base.Buffer);
  int X1 = Min(M.X, P.X + S.X * (D.X - 1)); /* last position */
  int X2 = P.X + S.X * (D.X - 2); /* second last position */
  int X3 = P.X + S.X * (D.X - 3); /* third last position */
  for (int Z = P.Z; Z < P.Z + S.Z * D.Z; Z += S.Z) {
    int ZZ = Min(Z, M.Z);
    for (int Y = P.Y; Y < P.Y + S.Y * D.Y; Y += S.Y) {
      int YY = Min(Y, M.Y);
      if (IsLastBlock) {
        /* extrapolate */
        bool Ext = IsEven(D.X);
        if (Ext) {
          t A = F[mg_RowX(X2, YY, ZZ, N)]; /* 2nd last (even) */
          t B = F[mg_RowX(X1, YY, ZZ, N)]; /* last (odd) */
          /* store the extrapolated value at the last odd position */
          //F[mg_RowX(X1, YY, ZZ, N)] = 2 * B - A;
          F[mg_RowX(M.X, YY, ZZ, N)] = 2 * B - A;
        }
        /* predict (excluding last odd position) */
        for (int X = P.X + S.X; X < P.X + S.X * (D.X - 2); X += 2 * S.X) {
          t & Val = F[mg_RowX(X, YY, ZZ, N)];
          Val -= F[mg_RowX(X - S.X, YY, ZZ, N)] / 2;
          Val -= F[mg_RowX(X + S.X, YY, ZZ, N)] / 2;
        }
        if (!Ext) { /* no extrapolation, predict at the last odd position */
          t & Val = F[mg_RowX(X2, YY, ZZ, N)];
          Val -= F[mg_RowX(X1, YY, ZZ, N)] / 2;
          Val -= F[mg_RowX(X3, YY, ZZ, N)] / 2;
        } else {
          F[mg_RowX(X1, YY, ZZ, N)] = 0;
        }
        /* update (excluding last odd position) */
        for (int X = P.X + S.X; X < P.X + S.X * (D.X - 2); X += 2 * S.X) {
          t Val = F[mg_RowX(X, YY, ZZ, N)];
          F[mg_RowX(X - S.X, YY, ZZ, N)] += Val / 4;
          F[mg_RowX(X + S.X, YY, ZZ, N)] += Val / 4;
        }
        if (!Ext) { /* no extrapolation, update at the last odd position */
          t Val = F[mg_RowX(X2, YY, ZZ, N)];
          F[mg_RowX(X1, YY, ZZ, N)] += Val / 4;
          F[mg_RowX(X3, YY, ZZ, N)] += Val / 4;
        }
      } else { /* not last block */
        /* predict (excluding the last odd position) */
        for (int X = P.X + S.X; X < P.X + S.X * (D.X - 2); X += 2 * S.X) {
          t & Val = F[mg_RowX(X, YY, ZZ, N)];
          Val -= F[mg_RowX(X - S.X, YY, ZZ, N)] / 2;
          Val -= F[mg_RowX(X + S.X, YY, ZZ, N)] / 2;
        }
        printf("\n");
        for (int I = 0; I < 10; ++I) { printf("%6.2f ", F[I]); }
        /* predict at the last odd position */
        t OVal = F[mg_RowX(X2, YY, ZZ, N)];
        OVal  -= F[mg_RowX(X1, YY, ZZ, N)] / 2;
        OVal  -= F[mg_RowX(X3, YY, ZZ, N)] / 2;
        /* update (excluding the last odd position */
        for (int X = P.X + S.X; X < P.X + S.X * (D.X - 2); X += 2 * S.X) {
          t Val = F[mg_RowX(X, YY, ZZ, N)];
          F[mg_RowX(X - S.X, YY, ZZ, N)] += Val / 4;
          F[mg_RowX(X + S.X, YY, ZZ, N)] += Val / 4;
        }
        //printf("\n");
        //for (int I = 0; I < 10; ++I) { printf("%6.2f ", F[I]); }
        /* update at the last odd position */
        if (!Overlap) {
          F[mg_RowX(X3, YY, ZZ, N)] += OVal / 4; /* update the 2nd last even */
          F[mg_RowX(X1, YY, ZZ, N)] += OVal / 4; /* update the last even */
          F[mg_RowX(X2, YY, ZZ, N)] = OVal; /* write the last odd */
        }
        //printf("\n");
        //for (int I = 0; I < 10; ++I) { printf("%6.2f ", F[I]); }
        int AA = 0;
      }
    }
  }
}
}
/* Forward lifting */
#define mg_FLiftCdf53(z, y, x)\
namespace mg {\
mg_T(t) void \
FLiftCdf53##x(grid_volume* Grid, const v3i& M, bool Overlap) {\
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);\
  if (D.x == 1) return;\
  mg_Assert(M.x <= N.x);\
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));\
  mg_Assert(D.x >= 2); /* TODO: what if D.x == 2? */\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 2) < M.x);\
  bool IsLastBlock = P.x + D.x * S.x >= M.x;\
  mg_Assert(IsLastBlock || IsOdd(D.x));\
  buffer_t<t> F(Grid->Base.Buffer);\
  int x1 = Min(P.x + S.x * (D.x - 1), M.x); /* last position */\
  int x2 = P.x + S.x * (D.x - 2); /* second last position */\
  int x3 = P.x + S.x * (D.x - 3); /* third last position */\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
    int zz = Min(z, M.z);\
    for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
      int yy = Min(y, M.y);\
      if (IsLastBlock) {\
        /* extrapolate */\
        bool Ext = IsEven(D.x);\
        if (Ext) {\
          t A = F[mg_Row##x(x2, yy, zz, N)]; /* 2nd last (even) */\
          t B = F[mg_Row##x(x1, yy, zz, N)]; /* last (odd) */\
          /* store the extrapolated value at the boundary position */\
          F[mg_Row##x(M.x, yy, zz, N)] = 2 * B - A;\
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
ILiftCdf53##x(grid_volume* Grid, const v3i& M, bool Overlap) {\
  v3i P = From(*Grid), D = Dims(*Grid), S = Strd(*Grid), N = Dims(Grid->Base);\
  if (D.x == 1) return;\
  mg_Assert(M.x <= N.x);\
  mg_Assert(IsPow2(S.X) && IsPow2(S.Y) && IsPow2(S.Z));\
  mg_Assert(D.x >= 2);\
  mg_Assert(IsEven(P.x));\
  mg_Assert(P.x + S.x * (D.x - 2) < M.x);\
  bool IsLastBlock = P.x + D.x * S.x >= M.x;\
  mg_Assert(IsLastBlock || IsOdd(D.x));\
  buffer_t<t> F(Grid->Base.Buffer);\
  int x1 = Min(P.x + S.x * (D.x - 1), M.x); /* last position */\
  int x2 = P.x + S.x * (D.x - 2); /* second last position */\
  int x3 = P.x + S.x * (D.x - 3); /* third last position */\
  for (int z = P.z; z < P.z + S.z * D.z; z += S.z) {\
    int zz = Min(z, M.z);\
    for (int y = P.y; y < P.y + S.y * D.y; y += S.y) {\
      int yy = Min(y, M.y);\
      if (IsLastBlock) {\
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
          t A = F[mg_Row##x(M.x, yy, zz, N)];\
          t B = F[mg_Row##x(x2, yy, zz, N)];\
          F[mg_Row##x(x1, yy, zz, N)] = A / 2 + B / 2;\
        }\
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

