// Testing run-length encoding of wavelet coefficients

#include "mg_array.h"
#include "mg_args.h"
#include "mg_bitops.h"
#include "mg_bitstream.h"
#include "mg_common.h"
#include "mg_io.h"
#include "mg_signal_processing.h"
#include "mg_timer.h"
#include "mg_volume.h"
#include "mg_zfp.h"
#include "mg_wavelet.h"
#include "mg_all.cpp"
#include <lz4/lz4.h>
#include <lz4/lz4_all.c>
#include <lz4/xxhash.c>
//#include <roaring/roaring.hh>
//#include <roaring/roaring.c>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace mg;
namespace chrono = std::chrono;

void TestZfp(volume& Vol, int NLevels, metadata Meta, const v3i& TileDims3, f64 Tolerance) {
  /* perform wavelet transform and allocate necessary buffers */
  //ForwardCdf53Old(&Vol, NLevels);
  array<extent> Sbands; BuildSubbands(Meta.Dims, NLevels, &Sbands);
  buffer Buf; AllocBuf(&Buf, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVol(Buf, TileDims3, Vol.Type);
  buffer BufQ; AllocBuf(&BufQ, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolQ(BufQ, TileDims3, IntType(Vol.Type)); // storing quantized values
  buffer BufN; AllocBuf(&BufN, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolN(BufN, TileDims3, IntType(UnsignedType(TileVolQ.Type))); // negabinary
  buffer BufO; AllocBuf(&BufO, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolO(BufO, TileDims3, TileVolN.Type); // decompression output
  buffer BsBuf; AllocBuf(&BsBuf, Vol.Buffer.Bytes);
  bitstream Bs; InitWrite(&Bs, BsBuf);
  buffer BsBuf2; AllocBuf(&BsBuf2, Vol.Buffer.Bytes);
  bitstream Bs2; InitWrite(&Bs2, BsBuf2);
  int SrcSizeMax = (Prod(TileDims3) + 7) / 8;
  int DstCapacity = LZ4_compressBound(SrcSizeMax);
  buffer DstBuf; AllocBuf(&DstBuf, DstCapacity);
  buffer DecompBuf; AllocBuf(&DecompBuf, SrcSizeMax);
  timer Timer;
  i64 TotalTime = 0;
  i64 TotalTime2 = 0;
  i64 TotalSize = 0;
  i64 TotalCompressedSize = 0;
  i64 TotalUncompressedSize = 0;
  i64 TotalTailBits = 0;

  /* encode the data */
  InitWrite(&Bs, Bs.Stream);
  InitWrite(&Bs2, Bs2.Stream);
  std::vector<bool> SigVec(Prod(TileDims3));
  v3i BlockDims3(4);
  v3i NBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
  array<i8> Ns; Init(&Ns, Prod(NBlocks3), i8(0));
  array<i8> Ms; Init(&Ms, Prod(NBlocks3), i8(0));
  array<i8> NOs; Init(&NOs, Prod(NBlocks3), i8(0));
  array<i8> MOs; Init(&MOs, Prod(NBlocks3), i8(0));
  v3i Dims3 = Dims(Vol);
  std::vector<bool> Check(Prod(Dims3), false);
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      std::fill(SigVec.begin(), SigVec.end(), false);
      Fill(Begin<u64>(TileVolO), End<u64>(TileVolO), 0);
      Fill(Begin<u64>(TileVol), End<u64>(TileVol), 0);
      v3i TileFrom3 = SbFrom3 + TileDims3 * T;
      v3i RealDims3 = Min(SbFrom3 + SbDims3 - TileFrom3, TileDims3);
      v3i P;
      // TODO: there is a bug if the tile is less than 32^3
      mg_Assert(RealDims3 == TileDims3);
      mg_BeginFor3(P, v3i::Zero, NBlocks3, v3i::One) { // through blocks
        int S = Row(NBlocks3, P) * Prod(BlockDims3);
        v3i D = P * BlockDims3 + TileFrom3;
        v3i B;
        mg_BeginFor3(B, v3i::Zero, BlockDims3, v3i::One) {
          TileVol.At<f64>(S + Row(BlockDims3, B)) = Vol.At<f64>(D + B);
          //mg_Assert(!Check[Row(Dims3, D + B)]);
          //Check[Row(Dims3, D + B)] = true;
        } mg_EndFor3
      } mg_EndFor3 // end sample loop
      /* quantize */
      int NBitplanes = Meta.Type == dtype::float32 ? 32 : 64;
      int EMax = Quantize(NBitplanes - 1, extent(RealDims3), TileVol, extent(RealDims3), &TileVolQ);
      /* do zfp transform */
      for (int B = 0; B < Prod(NBlocks3); ++B) {
        ForwardZfp((i64*)TileVolQ.Buffer.Data + B * Prod(BlockDims3));
        ForwardShuffle((i64*)TileVolQ.Buffer.Data + B * Prod(BlockDims3),
                       (u64*)TileVolN.Buffer.Data + B * Prod(BlockDims3));
      }
      /* extract bits */
      u64 Threshold = (Meta.Type == dtype::float32) ? (1 << 31) : (1ull << 63);
      int Bp = NBitplanes - 1;
      while ((Threshold > 0) && (NBitplanes - Bp <= EMax - Exponent(Tolerance) + 1)) { // through bit planes
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          Ms[B] = 0;
          Encode((u64*)TileVolN.Buffer.Data + B * Prod(BlockDims3), Bp, 1e9,
                 Ns[B], Ms[B], &Bs);
        }
        Flush(&Bs);
        TotalCompressedSize += Size(Bs);
        /* decompress here */
        StartTimer(&Timer);
        auto Start = chrono::steady_clock::now();
        InitRead(&Bs, Bs.Stream);
        int NSamples = Prod(RealDims3);
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          MOs[B] = 0;
          Decode((u64*)TileVolO.Buffer.Data + B * Prod(BlockDims3), Bp, 1e9,
                 NOs[B], MOs[B], &Bs);
        }
        TotalUncompressedSize += Size(Bs);
        InitWrite(&Bs, Bs.Stream);
        Threshold /= 2;
        --Bp;
        TotalTime += ElapsedTime(&Timer);
        auto End = chrono::high_resolution_clock::now();
        auto Diff = End - Start;
        auto Value = std::chrono::duration_cast<std::chrono::nanoseconds>(Diff);
        TotalTime += Value.count();
        ResetTimer(&Timer);
      } // end bit plane loop
    } mg_EndFor3 // end tile loop
  } // end subband loop
  /* write the bit stream */
  Flush(&Bs);
  printf("%lld %lld\n", TotalCompressedSize, TotalUncompressedSize);
  std::cout << (TotalTime / 1e6) << " ms" << std::endl;
}

void TestLz4(volume& Vol, int NLevels, metadata Meta, const v3i& TileDims3, f64 Tolerance) {
  /* perform wavelet transform and allocate necessary buffers */
  ForwardCdf53Old(&Vol, NLevels);
  array<extent> Sbands; BuildSubbands(Meta.Dims, NLevels, &Sbands);
  buffer Buf; AllocBuf(&Buf, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVol(Buf, TileDims3, Vol.Type);
  buffer BufQ; AllocBuf(&BufQ, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolQ(BufQ, TileDims3, IntType(Vol.Type)); // storing quantized values
  buffer BufN; AllocBuf(&BufN, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolN(BufN, TileDims3, IntType(UnsignedType(TileVolQ.Type))); // negabinary
  buffer BufO; AllocBuf(&BufO, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolO(BufO, TileDims3, TileVolN.Type); // decompression output
  buffer BsBuf; AllocBuf(&BsBuf, Vol.Buffer.Bytes);
  bitstream Bs; InitWrite(&Bs, BsBuf);
  buffer BsBuf2; AllocBuf(&BsBuf2, Vol.Buffer.Bytes);
  bitstream Bs2; InitWrite(&Bs2, BsBuf2);
  int SrcSizeMax = (Prod(TileDims3) + 7) / 8;
  int DstCapacity = LZ4_compressBound(SrcSizeMax);
  buffer DstBuf; AllocBuf(&DstBuf, DstCapacity);
  buffer DecompBuf; AllocBuf(&DecompBuf, SrcSizeMax);
  timer Timer;
  i64 TotalTime = 0;
  i64 TotalTime2 = 0;
  i64 TotalSize = 0;
  i64 TotalUncompressedSize = 0;
  i64 TotalTailBits = 0;
  //FILE* Fp = fopen("in.raw", "w");
  //FILE* Fp2 = fopen("out.raw", "w");

  /* encode the data */
  InitWrite(&Bs, Bs.Stream);
  InitWrite(&Bs2, Bs2.Stream);
  std::vector<bool> SigVec(Prod(TileDims3));
  v3i Dims3 = Dims(Vol);
  //Roaring R1;
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      std::fill(SigVec.begin(), SigVec.end(), false);
      //R1 = Roaring();
      //for (int I = 0; I < Prod(TileDims3); ++I) {
      //  R1.add(I);
      //}
      if (Meta.Type == dtype::float32)
        Fill(Begin<u32>(TileVolO), End<u32>(TileVolO), 0);
      else if (Meta.Type == dtype::float64)
        Fill(Begin<u64>(TileVolO), End<u64>(TileVolO), 0);
      v3i TileFrom3 = SbFrom3 + TileDims3 * T;
      v3i RealDims3 = Min(SbFrom3 + SbDims3 - TileFrom3, TileDims3);
      for (u32 Idx = 0; Idx < Prod<u32>(TileDims3); ++Idx) { // through tile samples
        v3i P(DecodeMorton3X(Idx), DecodeMorton3Y(Idx), DecodeMorton3Z(Idx));
        if (!(P < RealDims3)) // outside the domain
          continue;
        TileVol.At<f64>(P) = Vol.At<f64>(P + TileFrom3);
      } // end sample loop
      /* quantize */
      int NBitplanes = Meta.Type == dtype::float32 ? 32 : 64;
      int EMax = Quantize(NBitplanes - 1, extent(RealDims3), TileVol, extent(RealDims3), &TileVolQ);
      FwdNegaBinary(extent(RealDims3), TileVolQ, extent(RealDims3), &TileVolN);
      /* extract bits */
      u64 Threshold = (Meta.Type == dtype::float32) ? (1 << 31) : (1ull << 63);
      int Bp = NBitplanes - 1;
      while ((Threshold > 0) && (NBitplanes - Bp <= EMax - Exponent(Tolerance) + 1)) { // through bit planes
        for (int I = 0; I < Prod(RealDims3); ++I) { // through tile samples
          u64 Val = TileVolN.At<u64>(I);
          int AlreadySig = (Val >> 1) >= Threshold;
          if (!AlreadySig) {
            Write(&Bs, BitSet(Val, Bp));
          } else {
            Write(&Bs2, BitSet(Val, Bp));
          }
        } // end sample loop
        /* compress the significant map */
        Flush(&Bs);
        Flush(&Bs2);
        TotalUncompressedSize += Size(Bs);
        TotalTailBits += Size(Bs2);
        int CompressedSize =
          LZ4_compress_default((char*)Bs.Stream.Data, (char*)DstBuf.Data, Size(Bs), DstCapacity);
        /* decompress here */
        StartTimer(&Timer);
        auto Start = chrono::steady_clock::now();
        TotalSize += CompressedSize;
        int DecompressedSize =
          LZ4_decompress_safe((char*)DstBuf.Data, (char*)Bs.Stream.Data, CompressedSize, Bs.Stream.Bytes);
        auto End = chrono::high_resolution_clock::now();
        auto Diff = End - Start;
        auto Value = std::chrono::duration_cast<std::chrono::nanoseconds>(Diff);
        TotalTime2 += Value.count();
        InitRead(&Bs, Bs.Stream);
        InitRead(&Bs2, Bs2.Stream);
        int NSamples = Prod(RealDims3);
        u64 BitBuf = ReadLong(&Bs, 64);
        u64 BitBuf2 = ReadLong(&Bs2, 64);
        int J = 0, K = 0;
        for (int I = 0; I < NSamples; ++I) { // through tile samples
          u64& Val = TileVolO.At<u64>(I);
          if (SigVec[I]) {
            if (BitBuf2 & 1)
              Val |= 1ull << Bp;
            BitBuf2 >>= 1;
            if (K++ == 63) {
              BitBuf2 = ReadLong(&Bs2, 64);
              K = 0;
            }
            continue;
          }
          if (BitBuf & 1) {
            Val |= 1ull << Bp;
            SigVec[I] = true;
          }
          BitBuf >>= 1;
          if (J++ == 63) {
            BitBuf = ReadLong(&Bs, 64);
            J = 0;
          }
        } // end sample loop
        InitWrite(&Bs, Bs.Stream);
        InitWrite(&Bs2, Bs2.Stream);
        Threshold /= 2;
        --Bp;
        TotalTime += ElapsedTime(&Timer);
        auto End2 = chrono::high_resolution_clock::now();
        auto Diff2 = End2 - Start;
        auto Value2 = std::chrono::duration_cast<std::chrono::nanoseconds>(Diff2);
        TotalTime += Value2.count();
        ResetTimer(&Timer);
      } // end bit plane loop
    } mg_EndFor3 // end tile loop
  } // end subband loop
  /* write the bit stream */
  Flush(&Bs);
  Flush(&Bs2);
  printf("%lld %lld %lld\n", TotalTailBits, TotalUncompressedSize, TotalSize);
  std::cout << (TotalTime / 1e6) << " ms" << std::endl;
  std::cout << (TotalTime2 / 1e6) << " ms" << std::endl;
  //Err = WriteBuffer(OutputFile, buffer(Bs.Stream.Data, Size(Bs)));
  //mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  //Err = WriteBuffer("part2.raw", buffer(Bs2.Stream.Data, Size(Bs2)));
  //mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

int main(int Argc, const char** Argv) {
  /* Read data */
  cstr InputFile, OutputFile;
  mg_AbortIf(!OptVal(Argc, Argv, "--input", &InputFile), "Provide --input");
  mg_AbortIf(!OptVal(Argc, Argv, "--output", &OutputFile), "Provide --output");
  metadata Meta;
  error Err = ParseMeta(InputFile, &Meta);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  mg_AbortIf(Prod<i64>(Meta.Dims) > traits<i32>::Max, "Data dimensions too big");
  mg_AbortIf(Meta.Type != dtype::float32 && Meta.Type != dtype::float64,
             "Data type not supported");
  int NLevels;
  mg_AbortIf(!OptVal(Argc, Argv, "--num_levels", &NLevels), "Provide --num_levels");
  v3i TileDims3;
  mg_AbortIf(!OptVal(Argc, Argv, "--tile_dims", &TileDims3), "Provide --tile_dims");
  double Tolerance = 0;
  mg_AbortIf(!OptVal(Argc, Argv, "--tolerance", &Tolerance), "Provide --tolerance");
  volume Vol;
  Err = ReadVolume(InputFile, Meta.Dims, Meta.Type, &Vol);
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  volume VolCopy;
  Clone(Vol, &VolCopy);
  TestZfp(Vol, NLevels, Meta, TileDims3, Tolerance);
  TestLz4(VolCopy, NLevels, Meta, TileDims3, Tolerance);
}

