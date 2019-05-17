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
    volume Base(A, v3i(11, 1, 1));
    grid_volume Vol(v3i(10, 1, 1), Base);
    ForwardCdf53(&Vol, 1);
    InverseCdf53(&Vol, 1);
    for (int X = 0; X < Size(Vol); ++X) {
      mg_Assert(fabs(A[X] - B[X]) < 1e-9);
    }
  }
  { // 1D, three levels
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    volume Base(A, v3i(11, 1, 1));
    grid_volume Vol(v3i(10, 1, 1), Base);
    ForwardCdf53(&Vol, 3);
    InverseCdf53(&Vol, 3);
    for (int X = 0; X < Size(Vol); ++X) {
      mg_Assert(fabs(A[X] - B[X]) < 1e-9);
    }
  }
  { // 2D, one level
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    f64 C[11 * 11];
    grid_volume VolA(A, v3i(10, 10, 1));
    grid_volume VolB(B, v3i(10, 10, 1));
    grid_volume VolC(v3i(10, 10, 1), volume(C, v3i(11, 11, 1)));
    Copy(VolA, &VolC);
    ForwardCdf53(&VolC, 1);
    InverseCdf53(&VolC, 1);
    auto ItrC = Begin<f64>(VolC), ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // 2D, three levels
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    f64 C[11 * 11];
    grid_volume VolA(A, v3i(10, 10, 1));
    grid_volume VolB(B, v3i(10, 10, 1));
    grid_volume VolC(v3i(10, 10, 1), volume(C, v3i(11, 11, 1)));
    Copy(VolA, &VolC);
    ForwardCdf53(&VolC, 3);
    InverseCdf53(&VolC, 3);
    auto ItrC = Begin<f64>(VolC), ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // 3D, one level
    f64 A[] = Array7x6x5;
    f64 B[] = Array7x6x5;
    f64 C[8 * 7 * 6];
    grid_volume VolA(A, v3i(7, 6, 5));
    grid_volume VolB(B, v3i(7, 6, 5));
    grid_volume VolC(v3i(7, 6, 5), volume(C, v3i(8, 7, 6)));
    Copy(VolA, &VolC);
    ForwardCdf53(&VolC, 1);
    InverseCdf53(&VolC, 1);
    auto ItrC = Begin<f64>(VolC), ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // 3D, two levels
    f64 A[] = Array7x6x5;
    f64 B[] = Array7x6x5;
    f64 C[8 * 7 * 6];
    grid_volume VolA(A, v3i(7, 6, 5));
    grid_volume VolB(B, v3i(7, 6, 5));
    grid_volume VolC(v3i(7, 6, 5), volume(C, v3i(8, 7, 6)));
    Copy(VolA, &VolC);
    ForwardCdf53(&VolC, 2);
    InverseCdf53(&VolC, 2);
    auto ItrC = Begin<f64>(VolC), ItrB = Begin<f64>(VolB);
    for (; ItrC != End<f64>(VolC); ++ItrC, ++ItrB) {
      mg_Assert(fabs(*ItrC - *ItrB) < 1e-9);
    }
  }
  { // "real" 3d volume
    volume Vol(v3i(64, 64, 64), dtype::float64);
    ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[64-64-64]-Float64.raw",
               v3i(64, 64, 64), dtype::float64, &Vol);
    grid_volume Grid(Vol);
    volume VolCopy(v3i(65, 65, 65), dtype::float64);
    grid_volume GridCopy(v3i(64, 64, 64), VolCopy);
    Copy(Grid, &GridCopy);
    ForwardCdf53(&GridCopy, 5);
    InverseCdf53(&GridCopy, 5);
    volume_iterator It = Begin<f64>(Vol), VolEnd = End<f64>(Vol);
    grid_iterator CopyIt = Begin<f64>(GridCopy);
    for (; It != VolEnd; ++It, ++CopyIt) {
      mg_Assert(fabs(*It - *CopyIt) < 1e-9);
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

void
TestWaveletBlock() {
  { // 1D, one level
    f64 A[] = Array10x10;
    f64 B[] = Array10x10;
    grid_volume VolA(v3i(10, 1, 1), volume(A, v3i(11, 1, 1)));
    grid_volume VolB(v3i(10, 1, 1), volume(B, v3i(11, 1, 1)));
    ForwardCdf53(&VolA, 1);
    ForwardCdf53Block(&VolB, v3i(5, 1, 1), 1);
    grid_iterator ItA = Begin<f64>(VolA), VolEnd = End<f64>(VolA);
    grid_iterator ItB = Begin<f64>(VolB);
    printf("results\n");
    for (; ItA != VolEnd; ++ItA) {
      printf("%6.2f ", *ItA);
      //mg_Assert(fabs(*ItA - *ItB) < 1e-9);
    }
    printf("\n");
    for (ItA = Begin<f64>(VolA); ItA != VolEnd; ++ItA, ++ItB) {
      printf("%6.2f ", *ItB);
      //mg_Assert(fabs(*ItA - *ItB) < 1e-9);
    }
  }
  {
    //volume Vol(v3i(64, 64, 64), dtype::float64);
    //ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[64-64-64]-Float64.raw",
    //           v3i(64, 64, 64), dtype::float64, &Vol);
    //grid_volume Grid(Vol);
    //volume VolCopy(v3i(65, 65, 65), dtype::float64);
    //grid_volume GridCopy(v3i(64, 64, 64), VolCopy);
    //Copy(Grid, &GridCopy);
    //volume VolCopy2(v3i(65, 65, 65), dtype::float64);
    //grid_volume GridCopy2(v3i(64, 64, 64), VolCopy2);
    //Copy(Grid, &GridCopy2);
    //v3i BlockSize(33);
    //ForwardCdf53Block(&GridCopy, BlockSize, 1);
    //ForwardCdf53(&GridCopy2, 1);
    ////InverseCdf53(&GridCopy, 5);
    //volume_iterator It  = Begin<f64>(VolCopy), VolEnd = End<f64>(VolCopy);
    //volume_iterator It2 = Begin<f64>(VolCopy2);
    //for (; It != VolEnd; ++It, ++It2) {
    //  mg_Assert(fabs(*It - *It2) < 1e-9);
    //}
  }
}

mg_RegisterTest(Wavelet_TestWavelet, TestWavelet)
mg_RegisterTestOnly(Wavelet_TestWaveletBlock, TestWaveletBlock)
