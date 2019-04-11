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
  Dealloc(&List);
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
  error Err = ParseMeta(P.DataFile, &P.Meta);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  mg_AbortIf(Prod<i64>(P.Meta.Dims) > Traits<i32>::Max,
    "Data dimensions too big");
  mg_AbortIf(P.Meta.Type != data_type::float32 &&
             P.Meta.Type != data_type::float64, "Data type not supported");
  if (P.Action == action::Encode) {
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

int MaxNBlocks = 1;

void OldEncode(int BlockBegin, int BlockEnd) {
  FILE* Fp = fopen("blocks.raw", "rb");
  dynamic_array<u64> InBuf;
  Init(&InBuf, MaxNBlocks * 64);
  fread(InBuf.Buffer.Data, sizeof(u64), MaxNBlocks * 64, Fp);
  fclose(Fp);
  bitstream Bs;
  AllocBuf(&Bs.Stream, 1000000);
  InitWrite(&Bs, Bs.Stream);
  dynamic_array<i8> Ns;
  Init(&Ns, BlockEnd - BlockBegin, (i8)0);
  Fp = fopen("method2.raw", "wb");
  for (int Bp = 63; Bp >= 0; --Bp) {
    for (int I = BlockBegin; I < BlockEnd; ++I) {
      EncodeBlockOriginal(&InBuf[I * 64], Bp, Ns[I - BlockBegin], &Bs);
    }
  }
  if (BitSize(Bs) > 0) {
    Flush(&Bs);
    fwrite(Bs.Stream.Data, Size(Bs), 1, Fp);
  }
}

void TestEncoder(int ChunkSize, int BlockBegin, int BlockEnd) {
  FILE* Fp = fopen("test.raw", "rb");
  dynamic_array<u64> InBuf;
  Init(&InBuf, MaxNBlocks * 64);
  dynamic_array<u64> OutBuf;
  Init(&OutBuf, Size(InBuf), 0ull);
  fread(InBuf.Buffer.Data, sizeof(u64), MaxNBlocks * 64, Fp);
  fclose(Fp);
  bitstream Bs;
  AllocBuf(&Bs.Stream, 10000000);
  InitWrite(&Bs, Bs.Stream);
  Fp = fopen("method1.raw", "wb");
  dynamic_array<i8> Ns;
  Init(&Ns, BlockEnd - BlockBegin, (i8)0);
  for (int Bp = 63; Bp >= 0; --Bp) {
    for (int I = BlockBegin; I < BlockEnd; ++I) {
      i8 M = 0;
      bool InnerLoop = false;
      bool FullyEncoded = false;
      do {
        FullyEncoded = EncodeBlock(&InBuf[I * 64], Bp, ChunkSize, Ns[I - BlockBegin], M, InnerLoop, &Bs);
        if (BitSize(Bs) == ChunkSize) {
          Flush(&Bs);
          fwrite(Bs.Stream.Data, ChunkSize / 8, 1, Fp);
          InitWrite(&Bs, Bs.Stream);
        }
      } while (!FullyEncoded);
    }
  }
  if (BitSize(Bs) > 0) {
    Flush(&Bs);
    fwrite(Bs.Stream.Data, Size(Bs), 1, Fp);
  }
  Fill(Begin(Ns), End(Ns), (i8)0);
  fclose(Fp);
  Fp = fopen("method1.raw", "rb");
  fread(Bs.Stream.Data, ChunkSize / 8, 1, Fp);
  InitRead(&Bs, Bs.Stream);
  printf("-------------------------- done -------------------\n");
  for (int Bp = 63; Bp >= 0; --Bp) {
    for (int I = BlockBegin; I < BlockEnd; ++I) {
      i8 M = 0;
      bool InnerLoop = false;
      bool FullyDecoded = false;
      do {
        FullyDecoded = DecodeBlock(&OutBuf[I * 64], Bp, ChunkSize, Ns[I - BlockBegin], M, InnerLoop, &Bs);
        if (BitSize(Bs) == ChunkSize) {
          fread(Bs.Stream.Data, ChunkSize / 8, 1, Fp);
          InitRead(&Bs, Bs.Stream);
        }
      } while (!FullyDecoded);
    }
  }
  i64 Error = 0;
  for (int I = BlockBegin; I < BlockEnd; ++I) {
    for (int J = 0; J < 64; ++J)
      Error += InBuf[I * 64 + J] - OutBuf[I * 64 + J];
  }
  printf("%llu\n", Error);
  DeallocBuf(&Bs.Stream);
}

// TODO: handle float/int/int64/etc
int main(int Argc, const char** Argv) {
  //int ChunkSize, BlockBegin, BlockEnd;
  //ToInt(Argv[1], &ChunkSize);
  //ToInt(Argv[2], &BlockBegin);
  //ToInt(Argv[3], &BlockEnd);
  //OldEncode(BlockBegin, BlockEnd);
  //TestEncoder(ChunkSize, BlockBegin, BlockEnd);
  //return 0;

  /* Read the parameters */
  SetHandleAbortSignals();
  params P = ParseParams(Argc, Argv);
  file_format Ff;
  SetWaveletTransform(&Ff, P.NLevels);
  SetTileDims(&Ff, P.TileDims);
  SetChunkBytes(&Ff, P.ChunkBytes);
  SetPrecision(&Ff, P.NBitPlanes);
  SetTolerance(&Ff, P.Tolerance);
  SetFileName(&Ff, P.CompressedFile);

  /* Read the raw file */
  volume F;
  AllocBuf(&F.Buffer, SizeOf(P.Meta.Type) * Prod(P.Meta.Dims));
  mg_CleanUp(0, DeallocBuf(&F.Buffer));
  error Err = ReadVolume(P.Meta.File, P.Meta.Dims, P.Meta.Type, &F);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));

  /* Perform the action */
  if (P.Action == action::Encode) {
    SetVolume(&Ff, F.Buffer.Data, BigDims(F), F.Type);
    ff_err FfErr = Encode(&Ff, P.Meta);
    mg_AbortIf(ErrorExists(FfErr), "%s", ToString(FfErr));
#if defined(mg_CollectStats)
    Log("stats_encode.txt");
#endif
  } else { // Decode
    ff_err FfErr = Decode(&Ff, &P.Meta);
    mg_AbortIf(ErrorExists(FfErr), "%s", ToString(FfErr));
    f64 Error = RMSError(F, Ff.Volume);
    f64 Psnr = PSNR(F, Ff.Volume);
    printf("RMSE = %e PSNR = %f\n", Error, Psnr);

#if defined(mg_CollectStats)
    Log("stats_decode.txt");
#endif
  }
  CleanUp(&Ff);
  //mg_Assert(_CrtCheckMemory());
  return 0;
}
