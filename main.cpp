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

mg_Enum(action, int, Encode, Decode)

struct params {
  action Action = action::Encode;
  cstr DataFile = nullptr;
  cstr OutFile = nullptr;
  int NLevels = 0;
  int NBitPlanes = 0;
  v3i TileDims = v3i(0, 0, 0);
  int ChunkBytes = 0;
  f64 Tolerance = 0;
  metadata Meta;
};

params ParseParams(int Argc, const char** Argv) {
  params P;
  if (OptionExists(Argc, Argv, "--encode"))
    P.Action = action::Encode;
  else if (OptionExists(Argc, Argv, "--decode"))
    P.Action = action::Decode;
  else
    mg_Abort("Provide either --encode or --decode");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--dataset", &P.DataFile),
    "Provide --dataset");
  error Err = ParseMeta(P.DataFile, &P.Meta);
  mg_AbortIf(ErrorOccurred(Err), "%s", ToString(Err));
  mg_AbortIf(Prod<i64>(P.Meta.Dims) > Traits<i32>::Max,
    "Data dimensions too big");
  mg_AbortIf(P.Meta.Type != data_type::float32 &&
             P.Meta.Type != data_type::float64, "Data type not supported");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &P.NLevels),
    "Provide --nlevels");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--tiledims", &P.TileDims),
    "Provide --tiledims");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--chunkbytes", &P.ChunkBytes),
    "Provide --chunkbytes");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &P.OutFile),
    "Provide --output");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nbits", &P.NBitPlanes),
    "Provide --nbits");
  mg_AbortIf(P.NBitPlanes > BitSizeOf(P.Meta.Type),
    "--nbits too large");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--tolerance", &P.Tolerance),
    "Provide --tolerance");
  return P;
}

// TODO: handle float/int/int64/etc
int main(int Argc, const char** Argv) {
  SetHandleAbortSignals();
  params P = ParseParams(Argc, Argv);
  /* Read the original function */
  volume F;
  AllocBuf(&F.Buffer, SizeOf(P.Meta.Type) * Prod(P.Meta.Dims));
  mg_CleanUp(0, DeallocBuf(&F.Buffer));
  error Err = ReadVolume(P.Meta.File, P.Meta.Dims, P.Meta.Type, &F);
  mg_AbortIf(ErrorOccurred(Err), "%s", ToString(Err));
  volume FCopy;
  Clone(&FCopy, F);
  /* Encode */
  file_format Ff;
  SetVolume(&Ff, F.Buffer.Data, Extract3Ints64(F.DimsCompact), F.Type);
  SetWaveletTransform(&Ff, P.NLevels);
  SetTileDims(&Ff, P.TileDims);
  SetChunkBytes(&Ff, P.ChunkBytes);
  SetPrecision(&Ff, P.NBitPlanes);
  SetTolerance(&Ff, P.Tolerance);
  SetFileName(&Ff, P.OutFile);
  ff_err FfErr;
  if (P.Action == action::Encode) {
    FfErr = Encode(&Ff);
#if defined(mg_CollectStats)
    Log("stats_encode.txt");
#endif
  } else {
    FfErr = Decode(&Ff);
#if defined(mg_CollectStats)
    Log("stats_decode.txt");
#endif
  }
  mg_AbortIf(ErrorOccurred(FfErr), "%s", ToString(FfErr));
  CleanUp(&Ff);
  //mg_Assert(_CrtCheckMemory());
  return 0;
}
