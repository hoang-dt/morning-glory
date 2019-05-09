#include <stdio.h>
#include "mg_test.h"
#include "mg_common.h"
#include "mg_volume.h"

using namespace mg;

void TestGridCollapse() {
  {
    extent Ext(v3i(1, 2, 3), v3i(2, 3, 4));
    grid Grid(v3i(1, 2, 3), v3i(4, 6, 8), v3i(2, 3, 4));
    grid<volume> GridVol = GridCollapse(GridVolume(Ext), GridVolume(Grid));
    mg_Assert(From(GridVol) == v3i(3, 8, 15));
    mg_Assert(Dims(GridVol) == v3i(2, 3, 4));
    mg_Assert(Strd(GridVol) == v3i(2, 3, 4));
  }
  {
    grid Grid(v3i(1, 2, 3), v3i(2, 3, 4), v3i(1, 2, 3));
    extent<volume*> Ext(v3i(1, 2, 3), v3i(8, 12, 16));
    f64 A[1];
    buffer Buf((byte*)A, sizeof(A));
    volume Vol(Buf, v3i(100, 100, 100), data_type::float64);
    Ext.Base = &Vol;
    grid<volume> GridVol = GridCollapse(GridVolume(Grid), GridVolume(Ext));
    mg_Assert(From(GridVol) == v3i(2, 4, 6));
    mg_Assert(Dims(GridVol) == v3i(2, 3, 4));
    mg_Assert(Strd(GridVol) == v3i(1, 2, 3));
    mg_Assert(GridVol.Base == Vol);
  }
  {
    extent Ext1(v3i(1, 2, 3), v3i(2, 3, 4));
    extent<volume> Ext2(v3i(1, 2, 3), v3i(8, 12, 16));
    f64 A[1];
    buffer Buf((byte*)A, sizeof(A));
    volume Vol(Buf, v3i(100, 100, 100), data_type::float64);
    Ext2.Base = Vol;
    grid<volume> GridVol = GridCollapse(GridVolume(Ext1), GridVolume(Ext2));
    mg_Assert(From(GridVol) == v3i(2, 4, 6));
    mg_Assert(Dims(GridVol) == v3i(2, 3, 4));
    mg_Assert(Strd(GridVol) == v3i(1, 1, 1));
    mg_Assert(GridVol.Base == Vol);
  }
}

void TestGridIterator() {
  {
    extent<volume> Ext(v3i(0, 0, 0), v3i(3, 3, 3));
    f64 A[] = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                20, 21, 22, 23, 24, 25, 26, 27 };
    buffer Buf((byte*)A, sizeof(A));
    volume Vol(Buf, v3i(3, 3, 3), data_type::float64);
    Ext.Base = Vol;
    grid<volume> GridVol = GridVolume(Ext);
    int I = 0;
    for (auto It = Begin<f64>(GridVol); It != End<f64>(GridVol); ++It) {
      mg_Assert(*It == I++);
    }
  }
}

mg_RegisterTest(Volume_GridIterator, TestGridIterator)
mg_RegisterTest(Volume_GridCollapse, TestGridCollapse)

