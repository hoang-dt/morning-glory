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
#include "mg_expected.h"
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
#include "mg_volume.h"
#include "mg_zfp.h"
#include "mg_wavelet.h"

using namespace mg;

struct MinMaxExp {
  int MinExp;
  int MaxExp;
};

expected<int> TestExpected(int M) {
  if (M == 0) {
    return 1;
  }
  return mg_Error(SizeTooSmall);
}

array<v3i, 2> BoundLevelTest(v3i N, int L) {
  v3i P(1 << L, 1 << L, 1 << L);
  v3i MM = (N + P - 1) / P;
  for (int I = 0; I < L; ++I) {
    N.X = ((N.X >> 1) << 1) + 1;
    N.Y = ((N.Y >> 1) << 1) + 1;
    N.Z = ((N.Z >> 1) << 1) + 1;
    N.X = (N.X + 1) >> 1;
    N.Y = (N.Y + 1) >> 1;
    N.Z = (N.Z + 1) >> 1;
  }
  v3i M = N;
  N.X = ((N.X >> 1) << 1) + 1;
  N.Y = ((N.Y >> 1) << 1) + 1;
  N.Z = ((N.Z >> 1) << 1) + 1;
  printf("M %d %d %d\n", M.X, M.Y, M.Z);
  printf("MM %d %d %d\n", MM.X, MM.Y, MM.Z);
  printf("N %d %d %d\n", N.X, N.Y, N.Z);
  //mg_Assert(MM == N);
  return array<v3i, 2>{ M, N };
}

MinMaxExp GetMinMaxExp(const f64* Data, i64 Size) {
  auto MM = MinMaxElement(Data, Data + Size,
                          [](auto A, auto B) { return fabs(A) < fabs(B); });
  MinMaxExp Result;
  frexp(*(MM.Min), &Result.MinExp);
  frexp(*(MM.Max), &Result.MaxExp);
  return Result;
}

// TODO: have an abstraction for typed buffer
// TODO: enforce error checking everywhere
// TODO: memory out-of-bound/leak detection

// TODO: handle float/int/int64/etc
int main(int Argc, const char** Argv) {
  SetHandleAbortSignals();
  timer Timer; StartTimer(&Timer);
  /* Read the original data from a file */
  cstr DataFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--dataset", &DataFile), "Provide --dataset");
  int NLevels = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &NLevels), "Provide --nlevels");
  metadata Meta;
  error Ok = ReadMetadata(DataFile, &Meta);
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  i64 NumSamples = Prod<i64>(Meta.Dims);
  volume OriginalF; // original function
  AllocateBuffer(&OriginalF.Buffer, SizeOf(Meta.DataType) * NumSamples);
  mg_CleanUp(0, DeallocateBuffer(&OriginalF.Buffer));
  Ok = ReadVolume(Meta.File, Meta.Dims, Meta.DataType, &OriginalF);
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  /* Extrapolate the volume to 2^N+1 in each dimension */
  sub_volume ExpandedF;
  v3i BigDims(NextPow2(Meta.Dims.X) + 1, NextPow2(Meta.Dims.Y) + 1, NextPow2(Meta.Dims.Z) + 1);
  int MaxDim = Max(Max(BigDims.X, BigDims.Y), BigDims.Z);
  BigDims = v3i(MaxDim, Meta.Dims.Y > 1 ? MaxDim : 1, Meta.Dims.Z > 1 ? MaxDim : 1);
  mg_Log(stderr, "Big dims: %d %d %d\n", BigDims.X, BigDims.Y, BigDims.Z);
  ExpandedF.Dims = Stuff3Ints(BigDims);
  ExpandedF.Extent = extent(Meta.Dims);
  ExpandedF.Type = OriginalF.Type;
  i64 NumSamplesBig = Prod<i64>(Extract3Ints(ExpandedF.Dims));
  AllocateBufferZero(&ExpandedF.Buffer, SizeOf(ExpandedF.Type) * NumSamplesBig);
  mg_CleanUp(1, DeallocateBuffer(&ExpandedF.Buffer));
  Copy(&ExpandedF, sub_volume(OriginalF));
  Cdf53ForwardExtrapolate(&ExpandedF, ExpandedF.Type);
  Cdf53InverseExtrapolate(&ExpandedF, ExpandedF.Type);
  sub_volume ExpandedFCopy;
  Clone(&ExpandedFCopy, ExpandedF);
  /* Compute the wavelet transform */
  cstr OutFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &OutFile), "Provide --output");
  int NBitplanes = 63;
  GetOptionValue(Argc, Argv, "--nbits", &NBitplanes);
  f64 Tolerance = 0;
  GetOptionValue(Argc, Argv, "--tolerance", &Tolerance);
  Cdf53Forward(&ExpandedF, NLevels, ExpandedF.Type);
  printf("Wavelet transform time: %lld ms\n", ResetTimer(&Timer));
  /* Compress and write output files */
  dynamic_array<extent> Subbands;
  int NDims = (Meta.Dimensions.X > 1) + (Meta.Dimensions.Y > 1) + (Meta.Dimensions.Z > 1);
  BuildSubbands(NDims, BigDims, NLevels, &Subbands);
  buffer CompressBuf;
  AllocateBuffer(&CompressBuf, 1000 * 1000);
  v3i TileDims(32, 32, 32); // TODO: get from the command line
  EncodeData(ExpandedF, TileDims, NBitplanes, Tolerance, Subbands, OutFile);
  DecodeData(&ExpandedF, TileDims, NBitplanes, Tolerance, Subbands, OutFile);
  ////f64* FReconstructed = (f64*)BufFClone2.Data;
  ////if (Mode == 0)
    ////Decode(OutFile, Meta.Dims, TileDims, NBitplanes, Tolerance, Subbands, FReconstructed);
  ////else
    ////DecodeZfp(FReconstructed, Meta.Dims, TileDims, NBitplanes, Tolerance, Subbands, &Bs);
  ////Cdf53Inverse(FReconstructed, Meta.Dimensions, NLevels, Meta.DataType);
  //Cdf53InverseExtrapolate(F, Meta.Dimensions, NLevels, Meta.DataType);
  //f64 Psnr = PSNR(FClone, F, Prod<i64>(Meta.Dims), data_type::float64);
  //f64 Rmse = RMSError(FClone, F, Prod<i64>(Meta.Dims), data_type::float64);
  //printf("RMSE = %17g PSNR = %f\n", Rmse, Psnr);
  return 0;
}
