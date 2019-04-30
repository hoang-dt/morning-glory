#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <thread>
#include <vector>
#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/immediate_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <zfp.h>
#include <bitstream.h>
#include "../mg_args.h"
#include "../mg_assert.h"
#include "../mg_dataset.h"
#include "../mg_math.h"
#include "../mg_memory_map.h"
#include "../mg_timer.h"
#include "../mg_types.h"
#include "../mg_volume.h"
#include <omp.h>

// TODO: compile in Release mode

using namespace mg;
using namespace std;
using namespace stlab;

enum action { Encode, Decode };
struct params {
  action Action;
  cstr Input = nullptr;
  cstr Output = nullptr;
  int Rate = 1;
  metadata Meta;
};

params ParseParams(int Argc, const char** Argv) {
  params P;
  if (OptionExists(Argc, Argv, "--encode"))
    P.Action = Encode;
  else if (OptionExists(Argc, Argv, "--decode"))
    P.Action = Decode;
  else
    mg_Abort("Provide either --encode or --decode");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--input", &P.Input), "Provide --input");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &P.Output), "Provide --output");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--rate", &P.Rate), "Provide --rate");
  error Err = ParseMeta(P.Input, &P.Meta);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  return P;
}

void CopyBlockIn(f64* Dst, const f64* Src, v3i Dims, v3i Pos) {
  for (int Z = 0; Z < 4; ++Z) {
    for (int Y = 0; Y < 4; ++Y) {
      for (int X = 0; X < 4; ++X) {
        Dst[Z * 16 + Y * 4 + X] = Src[XyzToI(Dims, Pos + v3i(X, Y, Z))];
      }
    }
  }
}

void CopyBlockOut(f64* Dst, v3i Dims, v3i Pos, const f64* Src) {
  for (int Z = 0; Z < 4; ++Z) {
    for (int Y = 0; Y < 4; ++Y) {
      for (int X = 0; X < 4; ++X) {
        Dst[XyzToI(Dims, Pos + v3i(X, Y, Z))] = Src[Z * 16 + Y * 4 + X];
      }
    }
  }
}

static i64 Counter;
static mutex Mutex;
static condition_variable Cond;

void Compress(const params& P) {
  /* mmap the input file */
  mmap_file MMapIn; auto Err = open_file(P.Input, map_mode::Read, &MMapIn);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = map_file(&MMapIn); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));

  /* mmap the output file */
  mmap_file MMapOut;
  Err = open_file(P.Output, map_mode::Write, &MMapOut);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  v3i ZDims(4, 4, 4);
  v3i NumBlocks3 = (P.Meta.Dims + (ZDims - 1)) / ZDims;
  i64 TotalCompressedBytes = Prod<i64>(NumBlocks3) * 8 * P.Rate;
  Err = map_file(&MMapOut, TotalCompressedBytes);

  zfp_field* Field = zfp_field_3d(NULL, zfp_type_double, 4, 4, 4);
  zfp_stream* Zfp = zfp_stream_open(NULL);
  zfp_stream_set_rate(Zfp, P.Rate, zfp_field_type(Field), zfp_field_dimensionality(Field), 0);
  size_t Bytes = zfp_stream_maximum_size(Zfp, Field);
  zfp_stream ZfpCopy = *Zfp;
  bitstream* Stream = stream_open(MMapOut.Buf.Data, Bytes * (size_t)Prod<i64>(NumBlocks3));
  bitstream StreamCopy = *Stream;
  zfp_stream_set_bit_stream(Zfp, Stream);
  Counter = Prod<i64>(NumBlocks3);
  for (i64 BlockId = 0; BlockId < Prod<i64>(NumBlocks3); ++BlockId) { // for each block
    auto Fut = async(default_executor, [&, ZfpCopy, StreamCopy, BlockId]() mutable {
      v3i V = IToXyz(BlockId, NumBlocks3) * ZDims;
      f64 Block[64];
      CopyBlockIn(Block, (f64*)MMapIn.Buf.Data, P.Meta.Dims, V);
      zfp_stream_set_bit_stream(&ZfpCopy, &StreamCopy);
      stream_wseek(&StreamCopy, size_t(P.Rate * BlockId * 64));
      zfp_encode_block_double_3(&ZfpCopy, Block);
      zfp_stream_flush(&ZfpCopy);
      f64 Block2[64] = {};
      {
        unique_lock<mutex> Lock(Mutex);
        --Counter;
      }
      if (Counter == 0)
        Cond.notify_all();
    });
    Fut.detach();
  }
  unique_lock<mutex> Lock(Mutex);
  Cond.wait(Lock, []{ return Counter == 0; });
  zfp_field_free(Field);
  zfp_stream_close(Zfp);
  stream_close(Stream);
  //this_thread::sleep_for(chrono::milliseconds(10000));
  printf("Flusing file...\n");
  Err = flush_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = sync_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMapIn); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMapIn); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

void Decompress(const params& P) {
  timer Timer;
  StartTimer(&Timer);
  /* mmap the input file */
  mmap_file MMapIn; auto Err = open_file(P.Input, map_mode::Read, &MMapIn);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = map_file(&MMapIn); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));

  /* mmap the output file */
  mmap_file MMapOut;
  Err = open_file(P.Output, map_mode::Write, &MMapOut);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  v3i ZDims(4, 4, 4);
  v3i NumBlocks3 = (P.Meta.Dims + (ZDims - 1)) / ZDims;
  i64 TotalOutBytes = Prod<i64>(NumBlocks3) * 8 * 64;
  Err = map_file(&MMapOut, TotalOutBytes);

  zfp_field* Field = zfp_field_3d(NULL, zfp_type_double, 384, 384, 256);
  zfp_stream* Zfp = zfp_stream_open(NULL);
  zfp_stream_set_rate(Zfp, P.Rate, zfp_field_type(Field), zfp_field_dimensionality(Field), 0);
  bitstream* Stream = stream_open(MMapIn.Buf.Data, size_t(MMapIn.Buf.Bytes));
  int Step = 64;
  Counter = (Prod<i64>(NumBlocks3) + Step - 1) / Step;
  #pragma omp parallel for
  for (i64 BlockId = 0; BlockId < Prod<i64>(NumBlocks3); BlockId += Step) { // for each block
    //auto Fut = async(default_executor, [&, ZfpCopy, StreamCopy, BlockId]() mutable {
      for (i64 BId = BlockId; BId < BlockId + Step; ++BId) {
        zfp_stream ZfpCopy = *Zfp;
        bitstream StreamCopy = *Stream;
        v3i V = IToXyz(BId, NumBlocks3) * ZDims;
        f64 Block[64] = {};
        zfp_stream_set_bit_stream(&ZfpCopy, &StreamCopy);
        stream_rseek(&StreamCopy, size_t(P.Rate * BId * 64));
        zfp_decode_block_double_3(&ZfpCopy, Block);
        CopyBlockOut((f64*)MMapOut.Buf.Data, P.Meta.Dims, V, Block);
      }
      {
        //unique_lock<mutex> Lock(Mutex);
        //--Counter;
      }
      if (Counter == 0)
        Cond.notify_all();
    //});
    //Fut.detach();
  }
  unique_lock<mutex> Lock(Mutex);
  //Cond.wait(Lock, []{ return Counter == 0; });
  zfp_field_free(Field);
  zfp_stream_close(Zfp);
  stream_close(Stream);
  //this_thread::sleep_for(chrono::milliseconds(10000));
  auto Time = ResetTimer(&Timer);
  std::cout << (P.Action == Encode ? "Encode" : "Decode") << " time = " << Time << "ms\n";
  printf("Flusing file...\n");
  Err = flush_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = sync_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMapIn); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMapIn); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

int main(int Argc, const char** Argv) {
  auto P = ParseParams(Argc, Argv);
  if (P.Action == Encode) {
    Compress(P);
  } else if (P.Action == Decode) {
    Decompress(P);
  }
  //auto x = async(default_executor, [] { return 42; });

  //auto y = x.then([](int x) { printf("Result %d \n", x); });

  //// Waiting just for illustration purpose
  //while (!y.get_try()) { this_thread::sleep_for(chrono::milliseconds(1)); }
}
