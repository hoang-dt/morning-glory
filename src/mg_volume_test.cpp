#include <stdio.h>
#include "mg_test.h"
#include "mg_common.h"
#include "mg_volume.h"

using namespace mg;

void TestGridCollapse() {
  {
    extent Ext(v3i(1, 2, 3), v3i(2, 3, 4));
    grid Grid(v3i(1, 2, 3), v3i(4, 6, 8), v3i(2, 3, 4));
    grid GridCollapsed = GridCollapse(grid(Ext), Grid);
    mg_Assert(From(GridCollapsed) == v3i(3, 8, 15));
    mg_Assert(Dims(GridCollapsed) == v3i(2, 3, 4));
    mg_Assert(Strd(GridCollapsed) == v3i(2, 3, 4));
  }
  {
    grid Grid(v3i(1, 2, 3), v3i(2, 3, 4), v3i(1, 2, 3));
    f64 A[1];
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(100, 100, 100), data_type::float64);
    extent<volume*> Ext(v3i(1, 2, 3), v3i(8, 12, 16), &Vol);
    grid<volume*> GridCollapsed = GridCollapse(Grid, grid(Ext));
    mg_Assert(From(GridCollapsed) == v3i(2, 4, 6));
    mg_Assert(Dims(GridCollapsed) == v3i(2, 3, 4));
    mg_Assert(Strd(GridCollapsed) == v3i(1, 2, 3));
    mg_Assert(Value(GridCollapsed.Base) == Vol);
  }
  {
    extent Ext1(v3i(1, 2, 3), v3i(2, 3, 4));
    f64 A[1];
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(100, 100, 100), data_type::float64);
    extent<volume> Ext2(v3i(1, 2, 3), v3i(8, 12, 16), Vol);
    grid<volume> GridCollapsed = GridCollapse(grid(Ext1), grid(Ext2));
    mg_Assert(From(GridCollapsed) == v3i(2, 4, 6));
    mg_Assert(Dims(GridCollapsed) == v3i(2, 3, 4));
    mg_Assert(Strd(GridCollapsed) == v3i(1, 1, 1));
    mg_Assert(GridCollapsed.Base == Vol);
  }
}

void TestGridIterator() {
  {
    f64 A[] = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                20, 21, 22, 23, 24, 25, 26 };
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(3, 3, 3), data_type::float64);
    extent<volume> Ext(v3i(0, 0, 0), v3i(3, 3, 3), Vol);
    grid<volume> Grid(Ext);
    int I = 0;
    for (auto It = Begin<f64>(Grid); It != End<f64>(Grid); ++It) {
      mg_Assert(*It == I++);
    }
  }
  {
    f64 A[] = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                20, 21, 22, 23, 24, 25, 26 };
    volume Vol(buffer((byte*)A, sizeof(A)), v3i(3, 3, 3), data_type::float64);
    grid<volume> Grid(v3i(0, 0, 0), v3i(3, 1, 2), v3i(1, 3, 2), Vol);
    f64 B[] = { 0, 1, 2, 18, 19, 20 };
    int I = 0;
    for (auto It = Begin<f64>(Grid); It != End<f64>(Grid); ++It) {
      mg_Assert(*It == B[I++]);
    }
  }
}

void TestGridCopy() {
  {
    f64 A[27] = {};
    f64 B[] = { 0, 1, 2, 18, 19, 20 };
    volume VolA(buffer((byte*)A, sizeof(A)), v3i(3, 3, 3), data_type::float64);
    volume VolB(buffer((byte*)B, sizeof(B)), v3i(3, 1, 2), data_type::float64);
    grid<volume> GridA(VolA);
    grid<volume> GridB(VolB);
    //Copy(&Grid, );
    //int I = 0;
    //for (auto It = Begin<f64>(Grid); It != End<f64>(Grid); ++It) {
    //  mg_Assert(*It == I++);
    //}
  }
}

mg_RegisterTest(Volume_GridIterator, TestGridIterator)
mg_RegisterTest(Volume_GridCollapse, TestGridCollapse)

