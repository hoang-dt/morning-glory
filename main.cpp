#include "mg_array.h"
#include "mg_args.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_bitstream.h"
#include "mg_common_types.h"
#include "mg_dataset.h"
#include "mg_encode.h"
#include "mg_enum.h"
#include "mg_error.h"
#include "mg_filesystem.h"
#include "mg_logger.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_signal_processing.h"
#include "mg_dataset.h"
#include "mg_timer.h"
#include "mg_types.h"
#include "mg_zfp.h"
#include "mg_wavelet.h"

using namespace mg;

struct MinMaxExp {
  int MinExp;
  int MaxExp;
};

MinMaxExp GetMinMaxExp(const f64* Data, i64 Size) {
  auto MM = MinMaxElement(Data, Data + Size, [](auto A, auto B) { return fabs(A) < fabs(B); });
  MinMaxExp Result;
  frexp(*(MM.Min), &Result.MinExp);
  frexp(*(MM.Max), &Result.MaxExp);
  return Result;
}

// TODO: have an abstraction for typed buffer
// TODO: enforce error checking everywhere
// TODO: logger
// TODO: memory out-of-bound/leak detection

// TODO: handle float/int/int64/etc
int main(int Argc, const char** Argv) {
  SetHandleAbortSignals();
  timer Timer;
  StartTimer(&Timer);
  cstr DataFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--dataset", &DataFile), "Provide --dataset");
  metadata Meta;
  error Ok = ReadMetadata(DataFile, &Meta);
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  buffer BufF; // this stores the original function
  Ok = ReadFile(Meta.File, &BufF);
  buffer BufFClone = Clone(BufF);
  buffer BufFClone2 = Clone(BufF);
  // mg_CleanUp(0, Deallocate(&BufF.Data)); // TODO
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  puts("Done reading file\n");
  f64* F = (f64*)BufF.Data;
  f64* FClone = (f64*)BufFClone.Data;
  // mg_CleanUp(1, Deallocate(&BufFClone.Data))
  // mg_CleanUp(2, Deallocate(&BufFClone2.Data));
  buffer CompressBuf;
  AllocateBuffer(&CompressBuf, 400 * 1000 * 1000);
  //mg_CleanUp(3, Deallocate(&BufFClone3.Data));
  // TODO: copy F and perform inverse wavelet transform
  mg_AbortIf(Prod<i64>(Meta.Dims) * SizeOf(Meta.DataType) != BufF.Bytes, "Size mismatched. Check file: %s", Meta.File);
  int NLevels = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &NLevels), "Provide --nlevels");
  /* Compute wavelet transform */
  cstr OutFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &OutFile), "Provide --output");
  int NBitplanes = 63;
  GetOptionValue(Argc, Argv, "--nbits", &NBitplanes);
  f64 Tolerance = 0;
  GetOptionValue(Argc, Argv, "--tolerance", &Tolerance);
  int Mode = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--mode", &Mode), "Provide --mode");
  printf("Mode: %s Precision: %d Accuracy: %17g\n", Mode == 0 ? "Mine" : "Zfp", NBitplanes, Tolerance);
  Cdf53Forward(F, Meta.Dimensions, NLevels, Meta.DataType);
  printf("Wavelet transform time: %lld ms\n", ResetTimer(&Timer));
  dynamic_array<Block> Subbands;
  int NDims = (Meta.Dimensions.X > 1) + (Meta.Dimensions.Y > 1) + (Meta.Dimensions.Z > 1);
  BuildSubbands(NDims, Meta.Dimensions, NLevels, &Subbands);
  v3i TileDims{ 32, 32, 32 }; // TODO: get from the command line
  bitstream Bs;
  InitWrite(&Bs, CompressBuf);
  if (Mode == 0)
    Encode(F, Meta.Dims, TileDims, NBitplanes, Tolerance, Subbands, OutFile);
  else
    EncodeZfp(F, Meta.Dims, TileDims, NBitplanes, Tolerance, Subbands, &Bs);
  f64* FReconstructed = (f64*)BufFClone2.Data;
  if (Mode == 0)
    Decode(OutFile, Meta.Dims, TileDims, NBitplanes, Tolerance, Subbands, FReconstructed);
  else
    DecodeZfp(FReconstructed, Meta.Dims, TileDims, NBitplanes, Tolerance, Subbands, &Bs);
  Cdf53Inverse(FReconstructed, Meta.Dimensions, NLevels, Meta.DataType);
  f64 Psnr = PSNR(FClone, FReconstructed, Prod<i64>(Meta.Dims), data_type::float64);
  f64 Rmse = RMSError(FClone, FReconstructed, Prod<i64>(Meta.Dims), data_type::float64);
  printf("RMSE = %17g PSNR = %f\n", Rmse, Psnr);
  return 0;
}
