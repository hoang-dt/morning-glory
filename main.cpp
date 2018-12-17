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
  mg_CleanUp(0, Deallocate(&BufF.Data));
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  puts("Done reading file\n");
  f64* F = (f64*)BufF.Data;
  mg_AbortIf(Prod<i64>(Meta.Dims) * SizeOf(Meta.DataType) != BufF.Bytes, "Size mismatched. Check file: %s", Meta.File);
  int NLevels = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &NLevels), "Provide --nlevels");
  /* Compute wavelet transform */
  cstr OutFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &OutFile), "Provide --output");
  Cdf53Forward(F, Meta.Dimensions, NLevels, Meta.DataType);
  dynamic_array<Block> Subbands;
  int NDims = (Meta.Dimensions.X > 1) + (Meta.Dimensions.Y > 1) + (Meta.Dimensions.Z > 1);
  BuildSubbands(NDims, Meta.Dimensions, NLevels, &Subbands);
  v3i TileDims{ 32, 32, 32 }; // TODO: get from the command line
  Encode(F, Meta.Dims, TileDims, 64, Subbands, "out.raw");
  mg_HeapArray(FReconstructed, f64, sizeof(f64) * Prod<i64>(Meta.Dims));
  Decode("out.raw", Meta.Dims, TileDims, 64, Subbands, FReconstructed);
  f64 Ps = PSNR(F, FReconstructed, Prod<i64>(Meta.Dims), data_type::float64);
  printf("PSNR = %f\n", Ps);
  return 0;
}
