#include <cstddef>
#include <cstdio>
#include <thread>
#include <vector>
#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <zfp.h>
#include "../mg_args.h"
#include "../mg_assert.h"
#include "../mg_dataset.h"
#include "../mg_math.h"
#include "../mg_memory_map.h"
#include "../mg_types.h"
#include "../mg_volume.h"

using namespace mg;
using namespace std;
using namespace stlab;

struct params {
  cstr Input = nullptr;
  cstr Output = nullptr;
  int Rate = 1;
  metadata Meta;
};

params ParseParams(int Argc, const char** Argv) {
  params P;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--input", &P.Input), "Provide --input");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--output", &P.Output), "Provide --output");
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--raw_file", &P.Rate), "Provide --bitrate");
  error Err = ParseMeta(P.Input, &P.Meta);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  return P;
}

void CopyBlock(f64* Dst, const f64* Src, v3i Dims, v3i Pos) {
  for (int Z = 0; Z < 4; ++Z) {
    for (int Y = 0; Y < 4; ++Y) {
      for (int X = 0; X < 4; ++X) {
        Dst[Z * 16 + Y * 4 + X] = Src[XyzToI(Dims, Pos + v3i(X, Y, Z))];
      }
    }
  }
}

void Compress(const params& P) {
  /* mmap the input file */
  mmap_file MMapIn;
  auto Err = open_file(P.Input, map_mode::Read, &MMapIn);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = map_file(&MMapIn);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));

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
  zfp_field_free(Field);
  //mg_BeginFor3(V, v3i::Zero(), P.Meta.Dims, ZDims) {
  for (i64 BlockId = 0; BlockId < Prod<i64>(NumBlocks3); ++BlockId) { // for each block
    auto Fut = async(default_executor, [&]() {
      printf("Block %lld", BlockId);
      v3i V = IToXyz(BlockId, NumBlocks3) * ZDims;
      f64 Block[64];
      CopyBlock(Block, (f64*)MMapIn.Buf.Data, P.Meta.Dims, V);
      bitstream* Stream = stream_open(MMapOut.Buf.Data + BlockId * 8 * P.Rate, Bytes);
      zfp_stream_set_bit_stream(Zfp, Stream);
      zfp_encode_block_double_3(Zfp, Block);
      zfp_stream_flush(Zfp);
    });
  }
  Err = flush_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = sync_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = unmap_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = close_file(&MMapOut); mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

void decompress(const params& P, volume* Vol) {

}

int main(int Argc, const char** Argv) {
  auto P = ParseParams(Argc, Argv);

  //auto x = async(default_executor, [] { return 42; });

  //auto y = x.then([](int x) { printf("Result %d \n", x); });

  //// Waiting just for illustration purpose
  //while (!y.get_try()) { this_thread::sleep_for(chrono::milliseconds(1)); }
}
