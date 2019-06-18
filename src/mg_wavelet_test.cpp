#include "mg_test.h"

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
  { // 3D test
    f64 A[] = Array9x9x9;
    v3i M(9);
    volume VolA(A, M);
    ForwardCdf53Tile(1, v3i(4), &VolA);
  }
  { // 3D test
    volume Vol(v3i(64), dtype::float64);
    ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[64-64-64]-Float64.raw",
               v3i(64), dtype::float64, &Vol);
    ForwardCdf53Tile(2, v3i(4), &Vol);
  }
}

//mg_RegisterTest(Wavelet_TestWavelet, TestWavelet)
mg_RegisterTest(Wavelet_TestWaveletBlock, TestWaveletBlock)
