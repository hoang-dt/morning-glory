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
#include "mg_file_format.h"
#include "mg_filesystem.h"
#include "mg_linked_list.h"
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

// TODO: have an abstraction for typed buffer
// TODO: enforce error checking everywhere
// TODO: memory out-of-bound/leak detection

void TestLinkedList() {
  linked_list<int> List;
  auto Where1 = PushBack(&List, 1);
  auto Where2 = PushBack(&List, 2);
  auto Where3 = PushBack(&List, 3);
  auto Where4 = PushBack(&List, 4);
  Insert(&List, Where1, 5);
  Insert(&List, Where2, 6);
  Insert(&List, Where3, 7);
  Insert(&List, Where4, 8);
  for (auto It = ConstBegin(List); It != ConstEnd(List); ++It) {
    printf("%d\n", It->Payload);
  }
  Deallocate(&List);
}

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
  ExpandedF.DimsCompact = Stuff3Ints(BigDims);
  ExpandedF.Extent = extent(Meta.Dims);
  ExpandedF.Type = OriginalF.Type;
  i64 NumSamplesBig = Prod<i64>(Extract3Ints(ExpandedF.DimsCompact));
  AllocateBufferZero(&ExpandedF.Buffer, SizeOf(ExpandedF.Type) * NumSamplesBig);
  mg_CleanUp(1, DeallocateBuffer(&ExpandedF.Buffer));
  Copy(&ExpandedF, sub_volume(OriginalF));
  //Cdf53ForwardExtrapolate(&ExpandedF, ExpandedF.Type);
  //Cdf53InverseExtrapolate(&ExpandedF, ExpandedF.Type);
  //sub_volume ExpandedFCopy;
  //Clone(&ExpandedFCopy, ExpandedF);
  volume OriginalFCopy;
  Clone(&OriginalFCopy, OriginalF);
  /* Compute the wavelet transform */
  cstr OutFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &OutFile), "Provide --output");
  int NBitplanes = 63;
  GetOptionValue(Argc, Argv, "--nbits", &NBitplanes);
  f64 Tolerance = 0;
  GetOptionValue(Argc, Argv, "--tolerance", &Tolerance);
  /* Compress and write output files */
  dynamic_array<extent> Subbands;
  puts("Begin to encode");
  file_format FileData;
  SetVolume(&FileData, OriginalF.Buffer.Data, Extract3Ints(OriginalF.DimsCompact), OriginalF.Type);
  SetWaveletTransform(&FileData, NLevels);
  SetPrecision(&FileData, NBitplanes);
  SetTolerance(&FileData, Tolerance);
  SetFileName(&FileData, OutFile);
  Encode(&FileData);
  puts("Done encoding");
  //CleanUp(&FileData);
  //Cdf53Inverse(&ExpandedF, NLevels, ExpandedF.Type);
  //Cdf53Inverse(&OriginalF, NLevels);
  //f64 Psnr = PSNR(ExpandedF.Buffer.Data, ExpandedFCopy.Buffer.Data, ExpandedF.Dims,
                  //data_type::float64);
  //f64 Rmse = RMSError(ExpandedF.Buffer.Data, ExpandedFCopy.Buffer.Data, ExpandedF.Dims,
                      //data_type::float64);
  //f64 Psnr = PSNR(OriginalF.Buffer.Data, OriginalFCopy.Buffer.Data, Prod<i64>(Meta.Dims),
                  //data_type::float64);
  //f64 Rmse = RMSError(OriginalF.Buffer.Data, OriginalFCopy.Buffer.Data, Prod<i64>(Meta.Dims),
                      //data_type::float64);
  //printf("RMSE = %17g PSNR = %f\n", Rmse, Psnr);
  return 0;
}
