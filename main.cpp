#include "mg_array.h"
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

void TestArray() {
  stack_linear_allocator<1024> StackAlloc;
  fallback_allocator Alloc(&StackAlloc, &Mallocator());
  dynamic_array<Block> Subbands(&Alloc);
  BuildSubbands(3, v3l(8, 8, 8), 3, &Subbands);
  for (int I = 0; I < Size(Subbands); ++I) {
    v3i Pos = IToXyz(Subbands[I].Pos, v3l(8, 8, 8));
    printf("%d %d %d ", Pos.X, Pos.Y, Pos.Z);
    v3i Size = IToXyz(Subbands[I].Size, v3l(8, 8, 8));
    printf("%d %d %d\n", Size.X, Size.Y, Size.Z);
  }
}

int main(int Argc, const char** Argv) {
  SetHandleAbortSignals();
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
  mg_AbortIf(Size * SizeOf(Meta.DataType) != BufF.Bytes, "Size mismatched. Check file: %s", Meta.File);
  int NLevels = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &NLevels), "Provide --nlevels");
  buffer BufFWav;
  AllocateBuffer(&BufFWav, BufF.Bytes);
  f64* FWav = (f64*)BufFWav.Data;
  MemCopy(&BufFWav, BufF);
  mg_CleanUp(1, DeallocateBuffer(&BufFWav));
  // Cdf53Forward(FWav, Meta.Dimensions, NLevels, Meta.DataType);
  // Cdf53Inverse(FWav, Meta.Dimensions, NLevels, Meta.DataType);
  f64 Rmse = RMSError(FWav, F, Size);
  f64 Psnr = PSNR(FWav, F, Size);

  printf("Psnr = %f, Rmse = %f\n", Psnr, Rmse);
  printf("%" PRId64"\n", ElapsedTime(&Timer));

  i64* FQuant = (i64*)BufF.Data;
  int EMax = Quantize(F, Size, 32, FQuant, Meta.DataType);
  u64* FNega = (u64*)FQuant;
  data_type IntT = IntType(Meta.DataType);
  ConvertToNegabinary(FQuant, Size, FNega, IntT);
  ConvertFromNegabinary(FNega, Size, FQuant, IntT);
  Dequantize(FQuant, Size, EMax, 32, F, Meta.DataType);
  Rmse = RMSError(FWav, F, Size);
  Psnr = PSNR(FWav, F, Size);
  printf("Psnr = %f, Rmse = %f\n", Psnr, Rmse);
  // printf("Psnr = %f\n", Psnr);
}
