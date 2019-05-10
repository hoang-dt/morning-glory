#include "mg_signal_processing.h"
#include "mg_test.h"

using namespace mg;

void TestSqError() {
  f64 A[] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  f64 B[] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  f64 C[] = { 1e-12, 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  grid_volume GA(A, mg_ArraySize(A));
  grid_volume GB(B, mg_ArraySize(B));
  grid_volume GC(C, mg_ArraySize(C));
  f64 SqErr = SqError(GA, GB);
  mg_Assert(SqErr == 0);
  f64 Ps = PSNR(GA, GC);
  mg_Assert(Ps > 100);
}

void TestNegaBinary() {
  i64 A[] = { 0, 1001, 1001001, 1001001001, 1001001001001ll };
  i64 B[] = { 0, 1001, 1001001, 1001001001, 1001001001001ll };
  grid_volume GA(A, mg_ArraySize(A));
  grid_volume GB((u64*)A, mg_ArraySize(A));
  FwdNegaBinary(GA, &GB);
  InvNegaBinary(GB, &GA);
  int I = 0;
  for (auto It = Begin<i64>(GA); It != End<i64>(GA); ++It) {
    mg_Assert(*It == B[I++]);
  }
}

void TestQuantize() {
  f64 A[  ] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  f64 B[  ] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  grid_volume GA(A, mg_ArraySize(A));
  grid_volume GB((i64*)A, mg_ArraySize(A));
  int EMax = Quantize(GA, &GB, 52);
  Dequantize(GB, EMax, 52, &GA);
  int I = 0;
  for (auto It = Begin<f64>(GA); It != End<f64>(GA); ++It) {
    mg_Assert(fabs(*It - B[I++]) < 1e-15);
  }
}

mg_RegisterTest(SignalProcessing_SqError, TestSqError)
mg_RegisterTest(SignalProcessing_NegaBinary, TestNegaBinary)
mg_RegisterTest(SignalProcessing_Quantize, TestQuantize)
