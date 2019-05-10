#include "mg_array.h"
#include "mg_args.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_bitstream.h"
#include "mg_data_types.h"
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
#include "mg_memory_map.h"
#include "mg_scopeguard.h"
#include "mg_signal_processing.h"
#include "mg_dataset.h"
#include "mg_timer.h"
#include "mg_common.h"
#include "mg_volume.h"
#include "mg_zfp.h"
#include "mg_wavelet.h"
#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/default_executor.hpp>

using namespace mg;

// TODO: memory out-of-bound/leak detection

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
params ParseParams(int Argc, char** Argv) {
  params P;
  if (OptExists(Argc, Argv, "--encode"))
    P.Action = action::Encode;
  else if (OptExists(Argc, Argv, "--decode"))
    P.Action = action::Decode;
  else
    mg_Abort("Provide either --encode or --decode");
  mg_AbortIf(!OptVal(Argc, Argv, "--compressed_file", &P.CompressedFile),
    "Provide --compressed_files");
  mg_AbortIf(!OptVal(Argc, Argv, "--raw_file", &P.DataFile),
    "Provide --raw_file");
  error Err = ParseMeta(P.DataFile, &P.Meta);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  mg_AbortIf(Prod<i64>(P.Meta.Dims) > traits<i32>::Max,
    "Data dimensions too big");
  mg_AbortIf(P.Meta.Type != dtype::float32 &&
             P.Meta.Type != dtype::float64, "Data type not supported");
  if (P.Action == action::Encode) {
    mg_AbortIf(!OptVal(Argc, Argv, "--num_levels", &P.NLevels),
      "Provide --num_levels");
    mg_AbortIf(!OptVal(Argc, Argv, "--tile_dims", &P.TileDims),
      "Provide --tile_dims");
    mg_AbortIf(!OptVal(Argc, Argv, "--chunk_bytes", &P.ChunkBytes),
      "Provide --chunk_bytes");
    mg_AbortIf(!OptVal(Argc, Argv, "--precision", &P.NBitPlanes),
      "Provide --precision");
    mg_AbortIf(P.NBitPlanes > BitSizeOf(P.Meta.Type),
      "precision too high");
    mg_AbortIf(!OptVal(Argc, Argv, "--tolerance", &P.Tolerance),
      "Provide --tolerance");
  }
  return P;
}

void TestMemMap(char* Input) {
  /* test write */
  mmap_file MMap;
  auto Err = open_file("test.raw", map_mode::Write, &MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = map_file(&MMap, 128);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  MMap.Buf[4] = 'h';
  MMap.Buf[5] = 'e';
  MMap.Buf[6] = 'l';
  MMap.Buf[7] = 'l';
  MMap.Buf[8] = 'o';
  Err = flush_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = sync_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  /* test read */
  Err = open_file(Input, map_mode::Read, &MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = map_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  printf("%x\n", MMap.Buf[4096]);
  printf("%x\n", MMap.Buf[4096 * 2]);
  printf("%x\n", MMap.Buf[4096 * 3]);
  printf("%x\n", MMap.Buf[500000000]);
  Err = unmap_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMap);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

#define Array { 56, 40, 8, 24, 48, 48, 40, 16, 30, 32, 0, 0, 0,\
                40, 8, 24, 48, 48, 40, 16, 30, 32, 56, 0, 0, 0,\
                8, 24, 48, 48, 40, 16, 30, 32, 56, 40, 0, 0, 0,\
                24, 48, 48, 40, 16, 30, 32, 56, 40, 8, 0, 0, 0,\
                48, 48, 40, 16, 30, 32, 56, 40, 8, 24, 0, 0, 0,\
                48, 40, 16, 30, 32, 56, 40, 8, 24, 48, 0, 0, 0,\
                40, 16, 30, 32, 56, 40, 8, 24, 48, 48, 0, 0, 0,\
                16, 30, 32, 56, 40, 8, 24, 48, 48, 40, 0, 0, 0,\
                30, 32, 56, 40, 8, 24, 48, 48, 40, 16, 0, 0, 0,\
                32, 56, 40, 8, 24, 48, 48, 40, 16, 30, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }


void TestNewWaveletCode() {
  //double A[] = Array;
  //double C[] = Array;
  //double B[] = Array;
  //v3i NPow2(17, 17, 1);
  //v3i N(10, 10, 1);
  //int NLevels = 3;
  //v3i NLarge = ExpandDomain(N, NLevels);
  //double D[17 * 17 * 1] = {};
  //volume VolA(buffer(A), grid(NLarge), NLarge, data_type::float64);
  //volume VolD(buffer(D), grid(NLarge), NPow2, data_type::float64);
  //printf("%d %d %d\n", NLarge.X, NLarge.Y, NLarge.Z);
  //volume Vol(buffer(A), grid(N), NLarge, data_type::float64);
  //grid Ext = Vol.Extent;
  //volume Vol2 = Vol;
  //Vol2.Buffer = buffer(C);
  //array<grid> Extents, Extents2;
  //Resize(&Extents, NLevels);
  //Resize(&Extents2, NLevels);
  //printf("\n--------A:\n");
  //for (int Y = 0; Y < N.Y; ++Y) {
  //  for (int X = 0; X < N.X; ++X) {
  //    printf("%6.1f ", A[Y * NLarge.X + X]);
  //  }
  //  printf("\n");
  //}
  //v3i DimsP = NPow2;
  //v3i StridesP(1, 1, 1);
  //for (int I = 0; I < NLevels; ++I) {
  //  Extents2[I] = grid(v3i(0, 0, 0), DimsP, StridesP);
  //  DimsP = (DimsP + 1) / 2;
  //  StridesP = StridesP * 2;
  //}
  //for (int I = 0; I < NLevels; ++I) {
  //  FLiftCdf53X<double>(Vol, Ext);
  //  v3i Dims3 = Dims(Ext);
  //  Dims3.X += IsEven(Dims3.X);
  //  SetDims(&Ext, Dims3);
  //  FLiftCdf53Y<double>(Vol, Ext);
  //  Dims3.Y += IsEven(Dims3.Y);
  //  SetDims(&Ext, Dims3);
  //  Extents[I] = Ext;
  //  SetDims(&Ext, (Dims3 + 1) / 2);
  //  SetStrides(&Ext, Strides(Ext) * 2);
  //}
  //Copy(&VolD, VolA);
  //printf("inverse transform\n");
  //// inverse transform
  //for (int I = NLevels - 1; I >= 0; --I) {
  //  ILiftCdf53Y<double>(Vol, Extents[I]);
  //  printf("\n--------A (after Y pass):\n");
  //  for (int Y = 0; Y < NLarge.Y; ++Y) {
  //    for (int X = 0; X < NLarge.X; ++X) {
  //      printf("%6.1f ", A[Y * NLarge.X + X]);
  //    }
  //    printf("\n");
  //  }
  //  ILiftCdf53X<double>(Vol, Extents[I]);
  //  printf("\n--------A (after X pass):\n");
  //  for (int Y = 0; Y < NLarge.Y; ++Y) {
  //    for (int X = 0; X < NLarge.X; ++X) {
  //      printf("%6.1f ", A[Y * NLarge.X + X]);
  //    }
  //    printf("\n");
  //  }
  //  printf("-----------------------------------------\n");
  //}
  //printf("\n--------A:\n");
  //for (int Y = 0; Y < N.Y; ++Y) {
  //  for (int X = 0; X < N.X; ++X) {
  //    printf("%6.1f ", A[Y * NLarge.X + X]);
  //  }
  //  printf("\n");
  //}
  //printf("\n--------D (initially):\n");
  //for (int Y = 0; Y < NPow2.Y; ++Y) {
  //  for (int X = 0; X < NPow2.X; ++X) {
  //    printf("%6.1f ", D[Y * NPow2.X + X]);
  //  }
  //  printf("\n");
  //}
  //for (int I = NLevels - 1; I >= 0; --I) {
  //  ILiftUnpackCdf53Y<double>(VolD, Extents2[I], 0);
  //  printf("\n--------D (after Y pass):\n");
  //  for (int Y = 0; Y < NPow2.Y; ++Y) {
  //    for (int X = 0; X < NPow2.X; ++X) {
  //      printf("%6.1f ", D[Y * NPow2.X + X]);
  //    }
  //    printf("\n");
  //  }
  //  ILiftUnpackCdf53X<double>(VolD, Extents2[I], 1);
  //  printf("\n--------D (after X pass) :\n");
  //  for (int Y = 0; Y < NPow2.Y; ++Y) {
  //    for (int X = 0; X < NPow2.X; ++X) {
  //      printf("%6.1f ", D[Y * NPow2.X + X]);
  //    }
  //    printf("\n");
  //  }
  //  printf("-----------------------------------------\n");
  //}
  //printf("\n--------D:\n");
  //for (int Y = 0; Y < NPow2.Y; ++Y) {
  //  for (int X = 0; X < NPow2.X; ++X) {
  //    printf("%6.1f ", D[Y * NPow2.X + X]);
  //  }
  //  printf("\n");
  //}
  ////FormSubbands(&Vol2, Vol, NLevels);
}

#include <iostream>
#include <new>

// TODO: handle float/int/int64/etc
int main(int Argc, char** Argv) {
  TestNewWaveletCode();
  array<grid> Subbands;
  BuildSubbandsInPlace(v3i(16, 16, 16), 3, &Subbands);
  //for (int I = 0; I < Size(Subbands); ++I) {
  //  printf("----------- Subband %d\n", I);
  //  v3i Pos = Unpack3Ints64(Subbands[I].PosCompact);
  //  v3i Dims = Unpack3Ints64(Subbands[I].DimsCompact);
  //  v3i Strides = Unpack3Ints64(Subbands[I].StridesCompact);
  //  printf("Pos %d %d %d\n", Pos.X, Pos.Y, Pos.Z);
  //  printf("Dims %d %d %d\n", Dims.X, Dims.Y, Dims.Z);
  //  printf("Strides %d %d %d\n", Strides.X, Strides.Y, Strides.Z);
  //}
  return 0;
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
