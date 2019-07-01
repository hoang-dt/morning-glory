#include "mg_test.h"
#include "mg_common.h"
#include "mg_wavelet.h"
#include "mg_volume.h"
#include <math.h>

#define Array10x10\
 { 56, 40, 8, 24, 48, 48, 40, 16, 30, 32, 0,\
   40, 8, 24, 48, 48, 40, 16, 30, 32, 56, 0,\
   8, 24, 48, 48, 40, 16, 30, 32, 56, 40, 0,\
   24, 48, 48, 40, 16, 30, 32, 56, 40, 8, 0,\
   48, 48, 40, 16, 30, 32, 56, 40, 8, 24, 0,\
   48, 40, 16, 30, 32, 56, 40, 8, 24, 48, 0,\
   40, 16, 30, 32, 56, 40, 8, 24, 48, 48, 0,\
   16, 30, 32, 56, 40, 8, 24, 48, 48, 40, 0,\
   30, 32, 56, 40, 8, 24, 48, 48, 40, 16, 0,\
   32, 56, 40, 8, 24, 48, 48, 40, 16, 30, 0,\
    0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0 }

#define Array9x9\
 { 56, 40,  8, 24, 48, 48, 40, 16, 30,\
   40,  8, 24, 48, 48, 40, 16, 30, 32,\
    8, 24, 48, 48, 40, 16, 30, 32, 56,\
   24, 48, 48, 40, 16, 30, 32, 56, 40,\
   48, 48, 40, 16, 30, 32, 56, 40,  8,\
   48, 40, 16, 30, 32, 56, 40,  8, 24,\
   40, 16, 30, 32, 56, 40,  8, 24, 48,\
   16, 30, 32, 56, 40,  8, 24, 48, 48,\
   30, 32, 56, 40,  8, 24, 48, 48, 40 }

#define Array2_9x9\
 {  1,  2,  3,  4,  5,  6,  7,  8,  9,\
   10, 11, 12, 13, 14, 15, 16, 17, 18,\
   19, 20, 21, 22, 23, 24, 25, 26, 27,\
   28, 29, 30, 31, 32, 33, 34, 35, 36,\
   37, 38, 39, 40, 41, 42, 43, 44, 45,\
   46, 47, 48, 49, 50, 51, 52, 53, 54,\
   55, 56, 57, 58, 59, 60, 61, 62, 63,\
   64, 65, 66, 67, 68, 69, 70, 71, 72,\
   73, 74, 75, 76, 77, 78, 79, 80, 81 }

#define Array3_9x9\
 {  1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1,\
    1,  1,  1,  1,  1,  1,  1,  1,  1 }

#define Array9x9x9\
 { 56, 40,  8, 24, 48, 48, 40, 16, 30,  40,  8, 24, 48, 48, 40, 16, 30, 32,   8, 24, 48, 48, 40, 16, 30, 32, 56,\
   24, 48, 48, 40, 16, 30, 32, 56, 40,  48, 48, 40, 16, 30, 32, 56, 40,  8,  48, 40, 16, 30, 32, 56, 40,  8, 24,\
   40, 16, 30, 32, 56, 40,  8, 24, 48,  16, 30, 32, 56, 40,  8, 24, 48, 48,  30, 32, 56, 40,  8, 24, 48, 48, 40,\
                                                                                                                \
   40,  8, 24, 48, 48, 40, 16, 30, 32,   8, 24, 48, 48, 40, 16, 30, 32, 56,  24, 48, 48, 40, 16, 30, 32, 56, 40,\
   48, 48, 40, 16, 30, 32, 56, 40,  8,  48, 40, 16, 30, 32, 56, 40,  8, 24,  40, 16, 30, 32, 56, 40,  8, 24, 48,\
   16, 30, 32, 56, 40,  8, 24, 48, 48,  30, 32, 56, 40,  8, 24, 48, 48, 40,  56, 40,  8, 24, 48, 48, 40, 16, 30,\
                                                                                                                \
    8, 24, 48, 48, 40, 16, 30, 32, 56,  24, 48, 48, 40, 16, 30, 32, 56, 40,  48, 48, 40, 16, 30, 32, 56, 40,  8,\
   48, 40, 16, 30, 32, 56, 40,  8, 24,  40, 16, 30, 32, 56, 40,  8, 24, 48,  16, 30, 32, 56, 40,  8, 24, 48, 48,\
   30, 32, 56, 40,  8, 24, 48, 48, 40,  56, 40,  8, 24, 48, 48, 40, 16, 30,  40,  8, 24, 48, 48, 40, 16, 30, 32,\
                                                                                                                \
   24, 48, 48, 40, 16, 30, 32, 56, 40,  48, 48, 40, 16, 30, 32, 56, 40,  8,  48, 40, 16, 30, 32, 56, 40,  8, 24,\
   40, 16, 30, 32, 56, 40,  8, 24, 48,  16, 30, 32, 56, 40,  8, 24, 48, 48,  30, 32, 56, 40,  8, 24, 48, 48, 40,\
   56, 40,  8, 24, 48, 48, 40, 16, 30,  40,  8, 24, 48, 48, 40, 16, 30, 32,   8, 24, 48, 48, 40, 16, 30, 32, 56,\
                                                                                                                \
   48, 48, 40, 16, 30, 32, 56, 40,  8,  48, 40, 16, 30, 32, 56, 40,  8, 24,  40, 16, 30, 32, 56, 40,  8, 24, 48,\
   16, 30, 32, 56, 40,  8, 24, 48, 48,  30, 32, 56, 40,  8, 24, 48, 48, 40,  56, 40,  8, 24, 48, 48, 40, 16, 30,\
   40,  8, 24, 48, 48, 40, 16, 30, 32,   8, 24, 48, 48, 40, 16, 30, 32, 56,  24, 48, 48, 40, 16, 30, 32, 56, 40,\
                                                                                                                \
   48, 40, 16, 30, 32, 56, 40,  8, 24,  40, 16, 30, 32, 56, 40,  8, 24, 48,  16, 30, 32, 56, 40,  8, 24, 48, 48,\
   30, 32, 56, 40,  8, 24, 48, 48, 40,  56, 40,  8, 24, 48, 48, 40, 16, 30,  40,  8, 24, 48, 48, 40, 16, 30, 32,\
    8, 24, 48, 48, 40, 16, 30, 32, 56,  24, 48, 48, 40, 16, 30, 32, 56, 40,  48, 48, 40, 16, 30, 32, 56, 40,  8,\
                                                                                                                \
   40, 16, 30, 32, 56, 40,  8, 24, 48,  16, 30, 32, 56, 40,  8, 24, 48, 48,  30, 32, 56, 40,  8, 24, 48, 48, 40,\
   56, 40,  8, 24, 48, 48, 40, 16, 30,  40,  8, 24, 48, 48, 40, 16, 30, 32,   8, 24, 48, 48, 40, 16, 30, 32, 56,\
   24, 48, 48, 40, 16, 30, 32, 56, 40,  48, 48, 40, 16, 30, 32, 56, 40,  8,  48, 40, 16, 30, 32, 56, 40,  8, 24,\
                                                                                                                \
   16, 30, 32, 56, 40,  8, 24, 48, 48,  30, 32, 56, 40,  8, 24, 48, 48, 40,  56, 40,  8, 24, 48, 48, 40, 16, 30,\
   40,  8, 24, 48, 48, 40, 16, 30, 32,   8, 24, 48, 48, 40, 16, 30, 32, 56,  24, 48, 48, 40, 16, 30, 32, 56, 40,\
   48, 48, 40, 16, 30, 32, 56, 40,  8,  48, 40, 16, 30, 32, 56, 40,  8, 24,  40, 16, 30, 32, 56, 40,  8, 24, 48,\
                                                                                                                \
   30, 32, 56, 40,  8, 24, 48, 48, 40,  56, 40,  8, 24, 48, 48, 40, 16, 30,  40,  8, 24, 48, 48, 40, 16, 30, 32,\
    8, 24, 48, 48, 40, 16, 30, 32, 56,  24, 48, 48, 40, 16, 30, 32, 56, 40,  48, 48, 40, 16, 30, 32, 56, 40,  8,\
   48, 40, 16, 30, 32, 56, 40,  8, 24,  40, 16, 30, 32, 56, 40,  8, 24, 48,  16, 30, 32, 56, 40,  8, 24, 48, 48 }

#define Array7x6x5\
 { 56, 40,  8, 24, 48, 48, 40,   16, 30, 32, 40,  8, 24, 48,\
   48, 40, 16, 30, 32, 56,  8,   24, 48, 48, 40, 16, 30, 32,\
   56, 40, 24, 48, 48, 40, 16,   30, 32, 56, 40,  8, 48, 48,\
                                                            \
   40, 16, 30, 32, 56, 40,  8,   24, 48, 40, 16, 30, 32, 56,\
   40,  8, 24, 48, 40, 16, 30,   32, 56, 40,  8, 24, 48, 48,\
   16, 30, 32, 56, 40,  8, 24,   48, 48, 40, 30, 32, 56, 40,\
                                                            \
    8, 24, 48, 48, 40, 16, 32,   56, 40,  8, 24, 48, 48, 40,\
   16, 30, 56, 40,  8, 24, 48,   48, 40, 16, 30, 32, 40,  8,\
   24, 48, 48, 40, 16, 30, 32,   56,  8, 24, 48, 48, 40, 16,\
                                                            \
   30, 32, 56, 40, 24, 48, 48,   40, 16, 30, 32, 56, 40,  8,\
   48, 48, 40, 16, 30, 32, 56,   40,  8, 24, 48, 40, 16, 30,\
   32, 56, 40, 8, 24, 48,  40,   16, 30, 32, 56, 40,  8, 24,\
                                                            \
   48, 48, 16, 30, 32, 56, 40,    8, 24, 48, 48, 40, 30, 32,\
   56, 40,  8, 24, 48, 48, 40,   16, 32, 56, 40,  8, 24, 48,\
   48, 40, 16, 30, 56, 40,  8,   24, 48, 48, 40, 16, 30, 32 }

void
TestWavelet() {
  { // 1D, one level
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    volume Vol(A, v3i(11, 1, 1));
    grid Grid(v3i(10, 1, 1));
    ForwardCdf53(Grid, 1, &Vol);
    InverseCdf53(Grid, 1, &Vol);
    for (int X = 0; X < Size(Grid); ++X) {
      mg_Assert(fabs(A[X] - B[X]) < 1e-9);
    }
  }
  { // 1D, three levels
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    volume Vol(A, v3i(11, 1, 1));
    grid Grid(v3i(10, 1, 1));
    ForwardCdf53(Grid, 3, &Vol);
    InverseCdf53(Grid, 3, &Vol);
    for (int X = 0; X < Size(Grid); ++X) {
      mg_Assert(fabs(A[X] - B[X]) < 1e-9);
    }
  }
  { // 2D, one level
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    f64 C[11 * 11];
    volume VolA(A, v3i(10, 10, 1)), VolB(B, v3i(10, 10, 1)), VolC(C, v3i(11, 11, 1));
    grid Grid(v3i(10, 10, 1));
    Copy(Grid, VolA, Grid, &VolC);
    ForwardCdf53(Grid, 1, &VolC);
    InverseCdf53(Grid, 1, &VolC);
    auto ItrC = Begin<f64>(Grid, VolC);
    auto ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(Grid, VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // 2D, three levels
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    f64 C[11 * 11];
    volume VolA(A, v3i(10, 10, 1)), VolB(B, v3i(10, 10, 1)), VolC(C, v3i(11, 11, 1));
    grid Grid(v3i(10, 10, 1));
    Copy(Grid, VolA, Grid, &VolC);
    ForwardCdf53(Grid, 3, &VolC);
    InverseCdf53(Grid, 3, &VolC);
    auto ItrC = Begin<f64>(Grid, VolC);
    auto ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(Grid, VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // 3D, one level
    f64 A[] = Array7x6x5;
    f64 B[] = Array7x6x5;
    f64 C[8 * 7 * 6];
    volume VolA(A, v3i(7, 6, 5)), VolB(B, v3i(7, 6, 5)), VolC(C, v3i(8, 7, 6));
    grid Grid(v3i(7, 6, 5));
    Copy(Grid, VolA, Grid, &VolC);
    ForwardCdf53(Grid, 1, &VolC);
    InverseCdf53(Grid, 1, &VolC);
    auto ItrC = Begin<f64>(Grid, VolC);
    auto ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(Grid, VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // 3D, two levels
    f64 A[] = Array7x6x5;
    f64 B[] = Array7x6x5;
    f64 C[8 * 7 * 6];
    volume VolA(A, v3i(7, 6, 5)), VolB(B, v3i(7, 6, 5)), VolC(C, v3i(8, 7, 6));
    grid Grid(v3i(7, 6, 5));
    Copy(Grid, VolA, Grid, &VolC);
    ForwardCdf53(Grid, 2, &VolC);
    InverseCdf53(Grid, 2, &VolC);
    auto ItrC = Begin<f64>(Grid, VolC);
    auto ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(Grid, VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // "real" 3d volume
    volume Vol(v3i(64), dtype::float64);
    ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[64-64-64]-Float64.raw",
               v3i(64), dtype::float64, &Vol);
    volume VolExt(v3i(65), dtype::float64);
    extent Ext(v3i(64));
    Copy(Ext, Vol, &VolExt);
    ForwardCdf53(Ext, 5, &VolExt);
    InverseCdf53(Ext, 5, &VolExt);
    volume_iterator It = Begin<f64>(Vol), VolEnd = End<f64>(Vol);
    extent_iterator ItExt = Begin<f64>(Ext, VolExt);
    for (; It != VolEnd; ++It, ++ItExt) {
      mg_Assert(fabs(*It - *ItExt) < 1e-9);
    }
  }
  { // "real" 3d volume
    //volume Vol(v3i(384, 384, 256), dtype::float64);
    //ReadVolume("D:/Datasets/3D/Miranda/MIRANDA-DENSITY-[384-384-256]-Float64.raw",
    //           v3i(384, 384, 256), dtype::float64, &Vol);
    //grid_volume Grid(Vol);
    //volume VolCopy(v3i(385, 385, 257), dtype::float64);
    //grid_volume GridCopy(v3i(384, 384, 256), VolCopy);
    //Copy(Grid, &GridCopy);
    //ForwardCdf53(&GridCopy, 7);
    //InverseCdf53(&GridCopy, 7);
    //volume_iterator It = Begin<f64>(Vol), VolEnd = End<f64>(Vol);
    //grid_iterator CopyIt = Begin<f64>(GridCopy);
    //for (; It != VolEnd; ++It, ++CopyIt) {
    //  mg_Assert(fabs(*It - *CopyIt) < 1e-9);
    //}
  }
}

void TestWaveletBlock() {
  //{ // test with 2 tiles
  //  f64 A[] = Array9x9;
  //  f64 B[] = Array9x9;
  //  f64 C[] = Array9x9;
  //  f64 D[9 * 9] = {};
  //  volume VolA(A, v3i(9, 9, 1)), VolB(B, v3i(9, 9, 1));
  //  volume VolC(C, v3i(9, 9, 1)), VolD(D, v3i(9, 9, 1));
  //  extent ExtLeft(v3i(0, 0, 0), v3i(5, 9, 1));
  //  extent ExtRght(v3i(4, 0, 0), v3i(5, 9, 1));
  //  FLiftCdf53X<f64>(grid(ExtLeft), Dims(VolA), lift_option::PartialUpdateLast, &VolA);
  //  FLiftCdf53Y<f64>(grid(ExtLeft), Dims(VolA), lift_option::Normal, &VolA);
  //  FLiftCdf53X<f64>(grid(ExtRght), Dims(VolB), lift_option::Normal, &VolB);
  //  FLiftCdf53Y<f64>(grid(ExtRght), Dims(VolB), lift_option::Normal, &VolB);
  //  Add(ExtLeft, VolA, ExtLeft, &VolD);
  //  Add(ExtRght, VolB, ExtRght, &VolD);
  //  FLiftCdf53X<f64>(grid(Dims(VolC)), Dims(VolC), lift_option::Normal, &VolC);
  //  FLiftCdf53Y<f64>(grid(Dims(VolC)), Dims(VolC), lift_option::Normal, &VolC);
  //  auto ItrC = Begin<f64>(VolC);
  //  auto ItrD = Begin<f64>(VolD);
  //  for (ItrC = Begin<f64>(VolC); ItrC != End<f64>(VolC); ++ItrC, ++ItrD) {
  //    mg_Assert(fabs(*ItrC - *ItrD) < 1e-9);
  //  }
  //}
  { // 2D test
    //f64 A[] = Array9x9;
    //v3i M(9, 9, 1);
    //volume VolA(A, M);
    //volume VolB; Clone(VolA, &VolB);
    //Fill(Begin<f64>(VolB), End<f64>(VolB), 0);
    //ForwardCdf53Tile2D(1, v3i(4, 4, 1), VolA, &VolB);
    //ForwardCdf53Old(&VolA, 1);
    //for (auto ItA = Begin<f64>(VolA), ItB = Begin<f64>(VolB);
    //     ItA != End<f64>(VolA); ++ItA, ++ItB) {
    //  mg_Assert(*ItA == *ItB);
    //}
  }
  { // bigger 2D test
    v3i M(17, 17, 1);
    f64 A[M.X * M.Y];
    for (int Y = 0; Y < M.Y; ++Y) {
      f64 YY = (Y - M.Y / 2.0) / 2.0;
      f64 Vy = 3 / sqrt(2 * Pi) * exp(-0.5 * YY * YY);
      for (int X = 0; X < M.X; ++X) {
        f64 XX = (X - M.X / 2.0) / 2.0;
        f64 Vx = 3 / sqrt(2 * Pi) * exp(-0.5 * XX * XX);
        A[Y * 17 + X] = Vx * Vy;
      }
    }
    volume VolA(A, M);
    volume VolB; Clone(VolA, &VolB);
    Fill(Begin<f64>(VolB), End<f64>(VolB), 0);
    int NLevels = 1;
    v3i TDims3(4, 4, 1);
    ForwardCdf53Tile2D(NLevels, TDims3, VolA, &VolB);
    ForwardCdf53Old(&VolA, NLevels);
    array<extent> Sbands; BuildSubbands(M, NLevels, &Sbands);
    for (int Sb = 0; Sb < Size(Sbands); ++Sb) {
      v3i SbFrom3 = From(Sbands[Sb]);
      v3i SbDims3 = Dims(Sbands[Sb]);
      v3i NTiles3 = (SbDims3 + TDims3 - 1) / TDims3;
      v3i Tile;
      mg_BeginFor3(Tile, v3i::Zero, NTiles3, v3i::One) {
        v3i TFrom3 = SbFrom3 + Tile * TDims3;
        extent Ext3(TFrom3, Min(SbFrom3 + SbDims3 - TFrom3, TDims3));
        char FileName[256];
        sprintf(FileName, "A-sb-(%d)-tile-(%d-%d).txt", Sb, Tile.X, Tile.Y);
        DumpText(FileName, Begin<f64>(Ext3, VolA), End<f64>(Ext3, VolA), "%8.1e ");
      } mg_EndFor3
    }
    FILE* Fp = fopen("A.txt", "w");
    for (int Y = 0; Y < 17; ++Y) {
      for (int X = 0; X < 17; ++X) {
        fprintf(Fp, "%8.1e ", VolA.At<f64>(v3i(X, Y, 0)));
      }
      fprintf(Fp, "\n");
    }
    fclose(Fp);
    Fp = fopen("B.txt", "w");
    for (int Y = 0; Y < 17; ++Y) {
      for (int X = 0; X < 17; ++X) {
        fprintf(Fp, "%8.1e ", VolB.At<f64>(v3i(X, Y, 0)));
      }
      fprintf(Fp, "\n");
    }
    fclose(Fp);
  }
  //{ // 3D test
  //  printf("3D test\n");
  //  f64 A[] = Array9x9x9;
  //  v3i M(9);
  //  volume VolA(A, M);
  //  volume VolB; Clone(VolA, &VolB);
  //  Fill(Begin<f64>(VolB), End<f64>(VolB), 0);
  //  ForwardCdf53Tile(1, v3i(4), VolA, &VolB);
  //  //ForwardCdf53Old(&VolA, 1);
  //  DumpText("A.txt", Begin<f64>(VolA), End<f64>(VolA), "%f\n");
  //  DumpText("B.txt", Begin<f64>(VolB), End<f64>(VolB), "%f\n");
  //}
  { // 3D test
    //volume Vol(v3i(64), dtype::float64);
    //ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[64-64-64]-Float64.raw",
    //           v3i(64), dtype::float64, &Vol);
    //ForwardCdf53Tile(2, v3i(4), &Vol);
  }
}

mg_RegisterTest(Wavelet_TestWavelet, TestWavelet)
mg_RegisterTestOnly(Wavelet_TestWaveletBlock, TestWaveletBlock)
