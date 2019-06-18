#include <stdio.h>
#include "mg_test.h"
#include "mg_common.h"
#include "mg_volume.h"

using namespace mg;

void TestGridIterator() {
  {
    f64 A[] = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                20, 21, 22, 23, 24, 25, 26 };
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(3), dtype::float64);
    extent Ext(v3i::Zero, v3i(3));
    int I = 0;
    for (auto It = Begin<f64>(Ext, Vol); It != End<f64>(Ext, Vol); ++It) {
      mg_Assert(*It == A[I++]);
    }
  }
  {
    f64 A[] = {  0,  1,  2,  3,    4,  5,  6,  7,    8,  9, 10, 11,    12, 13, 14, 15,
                16, 17, 18, 19,   20, 21, 22, 23,   24, 25, 26, 27,    28, 29, 30, 31,
                32, 33, 34, 35,   36, 37, 38, 39,   40, 41, 42, 43,    44, 45, 46, 47,
                48, 49, 50, 51,   52, 53, 54, 55,   56, 57, 58, 59,    60, 61, 62, 63 };
    f64 B[] = { 21, 22, 25, 26, 37, 38, 41, 42 };
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(4), dtype::float64);
    extent Ext(v3i(1), v3i(2));
    int I = 0;
    for (auto It = Begin<f64>(Ext, Vol); It != End<f64>(Ext, Vol); ++It) {
      mg_Assert(*It == B[I++]);
    }
  }
  {
    f64 A[] = {  0,  1,  2,  3,    4,  5,  6,  7,    8,  9, 10, 11,    12, 13, 14, 15,
                16, 17, 18, 19,   20, 21, 22, 23,   24, 25, 26, 27,    28, 29, 30, 31,
                32, 33, 34, 35,   36, 37, 38, 39,   40, 41, 42, 43,    44, 45, 46, 47,
                48, 49, 50, 51,   52, 53, 54, 55,   56, 57, 58, 59,    60, 61, 62, 63 };
    f64 B[] = { 21, 23, 29, 31, 53, 55, 61, 63 };
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(4), dtype::float64);
    grid Grid(v3i(1), v3i(2), v3i(2));
    int I = 0;
    for (auto It = Begin<f64>(Grid, Vol); It != End<f64>(Grid, Vol); ++It) {
      mg_Assert(*It == B[I++]);
    }
  }
  {
    f64 A[] = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                20, 21, 22, 23, 24, 25, 26 };
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(3, 3, 3), dtype::float64);
    grid Grid(v3i::Zero, v3i(3, 1, 2), v3i(1, 3, 2));
    f64 B[] = { 0, 1, 2, 18, 19, 20 };
    int I = 0;
    for (auto It = Begin<f64>(Grid, Vol); It != End<f64>(Grid, Vol); ++It) {
      mg_Assert(*It == B[I++]);
    }
  }
}

mg_RegisterTest(Volume_GridIterator, TestGridIterator)

