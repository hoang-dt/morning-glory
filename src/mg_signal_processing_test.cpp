#include "mg_signal_processing.h"
#include "mg_test.h"

using namespace mg;

void TestSqError() {
  f64 A[] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  f64 B[] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  f64 C[] = { 1e-12, 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  volume GA(A, mg_ArraySize(A));
  volume GB(B, mg_ArraySize(B));
  volume GC(C, mg_ArraySize(C));
  f64 SqErr = SqError(extent(GA), GA, extent(GB), GB);
  mg_Assert(SqErr == 0);
  f64 Ps = PSNR(extent(GA), GA, extent(GC), GC);
  mg_Assert(Ps > 100);
}

void TestNegaBinary() {
  i64 A[] = { 0, 1001, 1001001, 1001001001, 1001001001001ll };
  i64 B[] = { 0, 1001, 1001001, 1001001001, 1001001001001ll };
  volume GA(A, mg_ArraySize(A));
  volume GB((u64*)A, mg_ArraySize(A));
  FwdNegaBinary(extent(GA), GA, extent(GB), &GB);
  InvNegaBinary(extent(GB), GB, extent(GA), &GA);
  int I = 0;
  for (auto It = Begin<i64>(GA); It != End<i64>(GA); ++It) {
    mg_Assert(*It == B[I++]);
  }
}

void TestQuantize() {
  f64 A[] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  f64 B[] = { 0e-6 , 1e-6, 2e-6, 3e-6, 4e-6, 5e-6, 6e-6, 7e-6, 8e-6, 9e-6 };
  volume GA(A, mg_ArraySize(A));
  volume GB((i64*)A, mg_ArraySize(A));
  int EMax = Quantize(52, extent(GA), GA, extent(GB), &GB);
  Dequantize(EMax, 52, extent(GB), GB, extent(GA), &GA);
  int I = 0;
  for (auto It = Begin<f64>(GA); It != End<f64>(GA); ++It) {
    mg_Assert(fabs(*It - B[I++]) < 1e-15);
  }
}

mg_RegisterTest(SignalProcessing_SqError, TestSqError)
mg_RegisterTest(SignalProcessing_NegaBinary, TestNegaBinary)
mg_RegisterTest(SignalProcessing_Quantize, TestQuantize)
