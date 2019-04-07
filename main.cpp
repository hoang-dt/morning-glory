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
  list<int> List;
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
  cstr CompressedFile = nullptr;
  int NLevels = 0;
  int NBitPlanes = 0;
  v3i TileDims = v3i(0, 0, 0);
  int ChunkBytes = 0;
  f64 Tolerance = 0;
  metadata Meta;
};

// TODO: when decoding, construct the raw file name from info embedded inside
// the compressed file
// TODO: add --precision for decoding
params ParseParams(int Argc, const char** Argv) {
  params P;
  if (OptionExists(Argc, Argv, "--encode"))
    P.Action = action::Encode;
  else if (OptionExists(Argc, Argv, "--decode"))
    P.Action = action::Decode;
  else
    mg_Abort("Provide either --encode or --decode");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--compressed_file", &P.CompressedFile),
    "Provide --compressed_files");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--raw_file", &P.DataFile),
    "Provide --raw_file");
  if (P.Action == action::Encode) {
    error Err = ParseMeta(P.DataFile, &P.Meta);
    mg_AbortIf(ErrorOccurred(Err), "%s", ToString(Err));
    mg_AbortIf(Prod<i64>(P.Meta.Dims) > Traits<i32>::Max,
      "Data dimensions too big");
    mg_AbortIf(P.Meta.Type != data_type::float32 &&
               P.Meta.Type != data_type::float64, "Data type not supported");
    mg_AbortIf(!GetOptionValue(Argc, Argv, "--num_levels", &P.NLevels),
      "Provide --num_levels");
    mg_AbortIf(!GetOptionValue(Argc, Argv, "--tile_dims", &P.TileDims),
      "Provide --tile_dims");
    mg_AbortIf(!GetOptionValue(Argc, Argv, "--chunk_bytes", &P.ChunkBytes),
      "Provide --chunk_bytes");
    mg_AbortIf(!GetOptionValue(Argc, Argv, "--precision", &P.NBitPlanes),
      "Provide --precision");
    mg_AbortIf(P.NBitPlanes > BitSizeOf(P.Meta.Type),
      "precision too high");
    mg_AbortIf(!GetOptionValue(Argc, Argv, "--tolerance", &P.Tolerance),
      "Provide --tolerance");
  }
  return P;
}

void TestEncoder() {
  FILE* Fp = fopen("blocks.raw", "rb");
  dynamic_array<u64> InBuf;
  Init(&InBuf, 14 * 64);
  dynamic_array<u64> OutBuf;
  Init(&OutBuf, Size(InBuf), 0ull);
  fread(InBuf.Buffer.Data, sizeof(u64), 14 * 64, Fp);
  fclose(Fp);
  bitstream Bs;
  AllocBuf(&Bs.Stream, 1000000);
  InitWrite(&Bs, Bs.Stream);
  i8 N = 0;
  for (int Bp = 63; Bp >= 0; --Bp) {
    i8 M = 0;
    bool InnerLoop = false;
    bool FullyEncoded = false;
    do {
      FullyEncoded = EncodeBlock(&InBuf[0], Bp, 1024, N, M, InnerLoop, &Bs);
      //FullyEncoded = EncodeBlock2(&InBuf[0], Bp, N, &Bs);
    } while (!FullyEncoded);
  }
  N = 0;
  Flush(&Bs);
  InitRead(&Bs, Bs.Stream);
  printf("-------------------------- done -------------------\n");
  for (int Bp = 63; Bp >= 0; --Bp) {
    i8 M = 0;
    bool InnerLoop = false;
    bool FullyDecoded = false;
    do {
      FullyDecoded = DecodeBlock(&OutBuf[0], Bp, 1024, N, M, InnerLoop, &Bs);
      //FullyDecoded = DecodeBlock2(&OutBuf[0], Bp, N, &Bs);
    } while (!FullyDecoded);
  }
  printf("--------- Input ---------\n");
  for (int I = 0; I < 64; ++I) {
    printf("%016llx\n", InBuf[I]);
  }
  printf("\n-------- Output --------\n");
  for (int I = 0; I < 64; ++I) {
    printf("%016llx\n", OutBuf[I]);
  }
  DeallocBuf(&Bs.Stream);
}

// TODO: handle float/int/int64/etc
int main(int Argc, const char** Argv) {
  TestEncoder();
  return 0;
  SetHandleAbortSignals();
  params P = ParseParams(Argc, Argv);
  file_format Ff;
  SetWaveletTransform(&Ff, P.NLevels);
  SetTileDims(&Ff, P.TileDims);
  SetChunkBytes(&Ff, P.ChunkBytes);
  SetPrecision(&Ff, P.NBitPlanes);
  SetTolerance(&Ff, P.Tolerance);
  SetFileName(&Ff, P.CompressedFile);
  if (P.Action == action::Encode) {
    volume F;
    AllocBuf(&F.Buffer, SizeOf(P.Meta.Type) * Prod(P.Meta.Dims));
    mg_CleanUp(0, DeallocBuf(&F.Buffer));
    error Err = ReadVolume(P.Meta.File, P.Meta.Dims, P.Meta.Type, &F);
    mg_AbortIf(ErrorOccurred(Err), "%s", ToString(Err));
    SetVolume(&Ff, F.Buffer.Data, Extract3Ints64(F.DimsCompact), F.Type);
    ff_err FfErr = Encode(&Ff, P.Meta);
    mg_AbortIf(ErrorOccurred(FfErr), "%s", ToString(FfErr));
#if defined(mg_CollectStats)
    Log("stats_encode.txt");
#endif
  } else {
    ff_err FfErr = Decode(&Ff, &P.Meta);
    mg_AbortIf(ErrorOccurred(FfErr), "%s", ToString(FfErr));
#if defined(mg_CollectStats)
    Log("stats_decode.txt");
#endif
  }
  CleanUp(&Ff);
  //mg_Assert(_CrtCheckMemory());
  return 0;
}
