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
    volume Base(A, v3i(11, 11, 1));
    grid_volume Vol(v3i(10, 10, 1), Base);
    ForwardCdf53(&Vol, 2);
    InverseCdf53(&Vol, 2);
    for (int X = 0; X < Size(Vol); ++X) {
      mg_Assert(fabs(A[X] - B[X]) < 1e-9);
    }
  }
  { // 2D, one level
    //f64 A[] = Array10x10;
    //f64 B[] = Array10x10;
    //grid_volume Vol(A, v3i(10, 10, 1));
    //ForwardCdf53(&Vol, 1);
    //InverseCdf53(&Vol, 1);
    //for (int I = 0; I < Size(Vol); ++I) {
    //  mg_Assert(fabs(A[I] - B[I]) < 1e-9);
    //}
  }
  { // 2D, three levels
    //f64 A[] = Array10x10;
    //f64 B[] = Array10x10;
    //grid_volume Vol(A, v3i(10, 10, 1));
    //ForwardCdf53(&Vol, 3);
    //InverseCdf53(&Vol, 3);
    //for (int I = 0; I < Size(Vol); ++I) {
    //  mg_Assert(fabs(A[I] - B[I]) < 1e-9);
    //}
  }
  { // 3D, one level
    //f64 A[] = Array7x6x5;
    //f64 B[] = Array7x6x5;
    //grid_volume Vol(A, v3i(7, 6, 5));
    //ForwardCdf53(&Vol, 1);
    //InverseCdf53(&Vol, 1);
    //for (int I = 0; I < Size(Vol); ++I) {
    //  mg_Assert(fabs(A[I] - B[I]) < 1e-9);
    //}
  }
  { // 3D, two levels
    f64 A[] = Array7x6x5;
    f64 B[] = Array7x6x5;
    grid_volume Vol(A, v3i(7, 6, 5));
    ForwardCdf53(&Vol, 2);
    InverseCdf53(&Vol, 2);
    for (int I = 0; I < Size(Vol); ++I) {
      mg_Assert(fabs(A[I] - B[I]) < 1e-9);
    }
  }
  { // "real" 3d volume
    //printf("volume\n");
    //volume Vol;
    //AllocBuf(&Vol.Buffer, sizeof(f64) * Prod<i64>(v3i(384, 384, 256)));
    //ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[64-64-64]-Float64.raw", 
    //           v3i(64, 64, 64), dtype::float64, &Vol);
    //volume VolCopy;
    //Clone(Vol, &VolCopy);
    //grid_volume Grid(Vol);
    //ForwardCdf53(&Grid, 2);
    //InverseCdf53(&Grid, 2);
    //volume_iterator VolIt = Begin<f64>(Vol), VolEnd = End<f64>(Vol);
    //volume_iterator VolCopyIt = Begin<f64>(VolCopy);
    //while (VolIt != VolEnd) {
    //  mg_Assert(fabs(*VolIt - *VolCopyIt) < 1e-6);
    //  ++VolIt; ++VolCopyIt;
    //}
  }
}

mg_RegisterTest(Wavelet_TestWavelet, TestWavelet)
