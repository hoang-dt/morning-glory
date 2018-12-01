#include "mg_args.h"
#include "mg_assert.h"
#include "mg_common_types.h"
#include "mg_dataset.h"
#include "mg_enum.h"
#include "mg_error.h"
#include "mg_filesystem.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_signal_processing.h"
#include "mg_dataset.h"
#include "mg_timer.h"
#include "mg_types.h"
#include "mg_wavelet.h"

using namespace mg;

void __attribute__ ((noinline)) B() {
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }

  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  mg_AbortIf(true);
}

int __attribute__ ((noinline)) A() {
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  for (int i = 0; i < 100; ++i) {
    if (i%50 == 0)
      printf("hohoho\n");
  }
  B();
}

int main(int Argc, const char** Argv) {
  SetHandleAbortSignals();
  A();
  timer Timer;
  StartTimer(&Timer);
  cstr DataFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--dataset", &DataFile), "Provide --dataset");
  metadata Meta;
  error Ok = ReadMetadata(DataFile, &Meta);
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  buffer BufF;
  Ok = ReadFile(Meta.File, &BufF);
  mg_CleanUp(0, Deallocate(&BufF.Data));
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  puts("Done reading file\n");
  f64* F = (f64*)BufF.Data;
  i64 Size = (i64)Meta.Dimensions.X * Meta.Dimensions.Y * Meta.Dimensions.Z;
  mg_AbortIf((size_t)Size * SizeOf(Meta.DataType) != BufF.Size, "Size mismatched. Check file: %s", Meta.File);
  int NLevels = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &NLevels), "Provide --nlevels");
  buffer BufFWav;
  mg_AbortIf(!AllocateBuffer(&BufFWav, BufF.Size), "Out of memory");
  f64* FWav = (f64*)BufFWav.Data;
  MemCopy(&BufFWav, BufF);
  mg_CleanUp(1, DeallocateBuffer(&BufFWav));
  Cdf53Forward(FWav, Meta.Dimensions, NLevels, Meta.DataType);
  Cdf53Inverse(FWav, Meta.Dimensions, NLevels, Meta.DataType);
  f64 Rmse = RMSError(FWav, F, Size);
  f64 Psnr = PSNR(FWav, F, Size);
  printf("Psnr = %f, Rmse = %f\n", Psnr, Rmse);
  printf("%" PRId64"\n", ElapsedTime(&Timer));
  // printf("Psnr = %f\n", Psnr);
}
