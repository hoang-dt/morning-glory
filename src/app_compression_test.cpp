// Testing run-length encoding of wavelet coefficients

// TODO: test 2D compression
// TODO: add speck
// TODO: try different transform (other than zfp)
// TODO: add a version of ReadVolume in which we can force the type

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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <lz4/lz4.h>
#include <lz4/lz4_all.c>
#include <lz4/xxhash.c>
#pragma clang diagnostic pop
//#include <roaring/roaring.hh>
//#include <roaring/roaring.c>
#include <chrono>
#include <openjp3d/openjp3d.h>
#include <openjp3d/openjpeg.h>

using namespace mg;
namespace chrono = std::chrono;

cstr DataFile_ = "D:/Datasets/3D/MARMOSET-NEURONS-[512-512-512]-Float32.raw";
int Prec_ = 16;
int NLevels_ = 3; // TODO: check if jpeg's nlevels is the same as our nlevels
int CBlock_ = 32; // size of the code block
v3i InputDims_(512, 512, 512);
dtype InputType_(dtype::float32);

void TestJp2k() {
  /* read the data from disk */
  volume Vol;
  ReadVolume(DataFile_, InputDims_, InputType_, &Vol);
  volume QVol(Dims(Vol), dtype::int32);
  v3i Dims3 = Dims(QVol);
  Quantize(Prec_, Vol, &QVol);
  //Clone(Vol ,&QVol);
  opj_cinfo_t* Compressor = opj_create_compress(CODEC_J3D);
  opj_cparameters_t Params;
  opj_set_default_encoder_parameters(&Params);
  Params.numresolution[2] = Params.numresolution[1] = Params.numresolution[0] = NLevels_ + 1;
  Params.cblock_init[0] =  Params.cblock_init[1] = Params.cblock_init[2] = CBlock_;
  Params.tile_size_on = true;
  Params.cp_tdx = Dims3.X; Params.cp_tdy = Dims3.Y; Params.cp_tdz = Dims3.Z;
  Params.prog_order = LRCP;
  Params.encoding_format = ENCOD_3EB;
  Params.transform_format = TRF_3D_DWT;
  // MAYBE: precint
  //Params.prct_init[3]
  //parameters->res_spec = res_spec;
  //parameters->csty |= 0x01;
  // TODO: set subsampling to 1 or 0
  if (Params.encoding_format == ENCOD_3EB)
    Params.mode |= (1 << 6);

  if (Params.tcp_numlayers == 0) {
    Params.tcp_rates[0] = 0.0;
    Params.tcp_numlayers++;
    Params.cp_disto_alloc = 1;
  }
  opj_volume_cmptparm_t VolParams;
  VolParams.dx = VolParams.dy = VolParams.dz = 1; // subsampling
  VolParams.w = Dims3.X; VolParams.h = Dims3.Y; VolParams.l = Dims3.Z;
  VolParams.x0 = VolParams.y0 = VolParams.z0 = 0;
  VolParams.prec = VolParams.bpp = Prec_;
  VolParams.sgnd = 1;
  VolParams.dcoffset = 0;
  VolParams.bigendian = 0;
  opj_volume_t* Volume = opj_volume_create(1, &VolParams, CLRSPC_GRAY);
  buffer CompBuf((byte*)Volume->comps[0].data, QVol.Buffer.Bytes);
  MemCopy(QVol.Buffer, &CompBuf);
  //Volume->comps[0].data = (int*)QVol.Buffer.Data;
  Volume->numcomps = 1;
  Volume->comps[0].bpp = Prec_;
  Volume->x0 = Volume->y0 = Volume->z0 = 0;
  Volume->x1 = VolParams.w;
  Volume->y1 = VolParams.h;
  Volume->z1 = VolParams.l;
  opj_setup_encoder(Compressor, &Params, Volume);
  opj_cio_t* Stream = opj_cio_open((opj_common_ptr)Compressor, nullptr, 0); // for writing
  opj_encode(Compressor, Stream, Volume, nullptr);
  opj_volume_destroy(Volume);
  /* decode */
  auto CompressedBuf = Stream->buffer;
  auto Length = cio_tell(Stream);
  //cio_seek(Stream, 0);
  opj_dinfo_t* Decompressor = opj_create_decompress(CODEC_J3D);
  opj_dparameters_t DParams;
  opj_set_default_decoder_parameters(&DParams); // TODO
  DParams.decod_format = J3D_CFMT;
  opj_setup_decoder(Decompressor, &DParams);
  opj_cio_t* DStream = opj_cio_open((opj_common_ptr)Decompressor, CompressedBuf, Length);
  opj_volume_t* OutVol = opj_decode(Decompressor, DStream);
  //FILE* Fp = fopen("decompressed.raw", "wb");
  //fwrite(OutVol->comps[0].data, Prod(Dims3) * sizeof(int), 1, Fp);
  //fclose(Fp);
  //OutVol->comps[0]->data;
  auto Psnr = PSNR(QVol, volume(OutVol->comps[0].data, Dims(QVol)));
  printf("psnr = %f\n", Psnr);
  DeallocBuf(&QVol.Buffer);
  opj_cio_close(Stream);
  opj_cio_close(DStream);
  opj_volume_destroy(OutVol);
  opj_destroy_compress(Compressor);
  opj_destroy_decompress(Decompressor);
}

void TestZfp2() {
  /* read the data from disk */
  v3i TileDims3(CBlock_);
  volume Vol;
  ReadVolume(DataFile_, InputDims_, InputType_, &Vol);
  volume QVol(Dims(Vol), dtype::int32);
  v3i Dims3 = Dims(QVol);
  Quantize(Prec_, Vol, &QVol);
  //Clone(Vol, &QVol);
  ForwardCdf53Old(&QVol, NLevels_);
  array<extent> Sbands; BuildSubbands(Dims(QVol), NLevels_, &Sbands);
  buffer BufQ; AllocBuf(&BufQ, SizeOf(QVol.Type) * Prod(TileDims3));
  volume TileVolQ(BufQ, TileDims3, QVol.Type); // quantized tile data
  buffer BufN; AllocBuf(&BufN, SizeOf(QVol.Type) * Prod(TileDims3));
  volume TileVolN(BufN, TileDims3, IntType(UnsignedType(TileVolQ.Type))); // negabinary
  buffer BufO; AllocBuf(&BufO, SizeOf(TileVolN.Type) * Prod(TileDims3));
  volume TileVolO(BufO, TileDims3, TileVolN.Type); // decompression output
  buffer BsBuf; AllocBuf(&BsBuf, Vol.Buffer.Bytes);
  bitstream Bs; InitWrite(&Bs, BsBuf);
  timer Timer;
  i64 TotalCompressionTime = 0;
  i64 TotalDecompressionTime = 0;
  i64 TotalCompressedSize = 0;
  /* -------- encode the data --------- */
  StartTimer(&Timer);
  InitWrite(&Bs, Bs.Stream);
  v3i BlockDims3(4);
  v3i NBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
  array<i8> Ns; Init(&Ns, Prod(NBlocks3), i8(0));
  //FILE* Fp = fopen("encode.raw", "wb");
  //FILE* Fp2 = fopen("encode.txt", "w");
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      Fill(Begin<u32>(TileVolO), End<u32>(TileVolO), 0);
      Fill(Begin<u32>(TileVolN), End<u32>(TileVolN), 0);
      Fill(Begin<i8>(Ns), End<i8>(Ns), 0);
      v3i TileFrom3 = SbFrom3 + TileDims3 * T;
      v3i RealDims3 = Min(SbFrom3 + SbDims3 - TileFrom3, TileDims3);
      v3i P3;
      // TODO: there is a bug if the tile is less than 32^3
      mg_Assert(RealDims3 == TileDims3);
      mg_BeginFor3(P3, v3i::Zero, NBlocks3, v3i::One) { // through blocks
        int S = Row(NBlocks3, P3) * Prod(BlockDims3);
        v3i D3 = P3 * BlockDims3 + TileFrom3;
        v3i B3;
        mg_BeginFor3(B3, v3i::Zero, BlockDims3, v3i::One) {
          TileVolQ.At<i32>(S + Row(BlockDims3, B3)) = QVol.At<i32>(D3 + B3);
        } mg_EndFor3
        //fwrite((i32*)TileVolQ.Buffer.Data + S, Prod(BlockDims3) * sizeof(i32), 1, Fp);
        //for (int I = 0; I < 64; ++I)
        //  fprintf(Fp2, "%d\n", *((i32*)TileVolQ.Buffer.Data + S + I));
        ForwardZfp((i32*)TileVolQ.Buffer.Data + S);
        for (int I = 0; I < 4 * 4 * 4; ++I) {
          i32 Val = *((i32*)TileVolQ.Buffer.Data + S + I);
          if (!(Val >= -(1 << 18) && Val < (1 << 18)))
            printf("!!!!!!! overflow !!!!!!!!\n");
        }
        ForwardShuffle((i32*)TileVolQ.Buffer.Data + S, (u32*)TileVolN.Buffer.Data + S);
      } mg_EndFor3 // end sample loop
      int NBitplanes = 20; //Prec + 1;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) { // through bit planes
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          u32* Beg = (u32*)TileVolN.Buffer.Data + B * Prod(BlockDims3);
          Encode<u32>(Beg, Bp, 2e9, Ns[B], &Bs);
        }
        --Bp;
      } // end bit plane loop
    } mg_EndFor3 // end tile loop
  } // end subband loop
  TotalCompressionTime += ElapsedTime(&Timer);
  Flush(&Bs);
  TotalCompressedSize += Size(Bs);
  //fclose(Fp);
  //fclose(Fp2);

  /* ---------- decode ---------- */
  //Fp = fopen("decode.raw", "wb");
  //Fp2 = fopen("decode.txt", "w");
  volume QVolBackup; Clone(QVol, &QVolBackup);
  InverseCdf53Old(&QVolBackup, NLevels_);
  Fill(Begin<i32>(QVol), End<i32>(QVol), 0);
  InitRead(&Bs, Bs.Stream);
  StartTimer(&Timer);
  array<i8> NOs; Init(&NOs, Prod(NBlocks3), i8(0));
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      Fill(Begin<u32>(TileVolO), End<u32>(TileVolO), 0);
      Fill(Begin<i8>(NOs), End<i8>(NOs), 0);
      v3i TileFrom3 = SbFrom3 + TileDims3 * T;
      v3i RealDims3 = Min(SbFrom3 + SbDims3 - TileFrom3, TileDims3);
      /* decompress here */
      int NBitplanes = 20;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) {
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          u32* Beg = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
          Decode<u32>(Beg, Bp, 2e9, NOs[B], &Bs);
        }
        --Bp;
      }
      v3i P3;
      mg_BeginFor3(P3, v3i::Zero, NBlocks3, v3i::One) { // through blocks
        int S = Row(NBlocks3, P3) * Prod(BlockDims3);
        v3i D3 = P3 * BlockDims3 + TileFrom3;
        InverseShuffle((u32*)TileVolO.Buffer.Data + S, (i32*)TileVolQ.Buffer.Data + S);
        InverseZfp((i32*)TileVolQ.Buffer.Data + S);
        //fwrite((i32*)TileVolQ.Buffer.Data + S, Prod(BlockDims3) * sizeof(i32), 1, Fp);
        //for (int I = 0; I < 64; ++I)
        //  fprintf(Fp2, "%d\n", *((i32*)TileVolQ.Buffer.Data + S + I));
        v3i B3;
        mg_BeginFor3(B3, v3i::Zero, BlockDims3, v3i::One) {
          QVol.At<i32>(D3 + B3) = TileVolQ.At<i32>(S + Row(BlockDims3, B3));
        } mg_EndFor3
      } mg_EndFor3
    } mg_EndFor3
  }
  auto DecompressedSize = Size(Bs);
  printf("decompressed size = %lld\n", DecompressedSize);
  TotalDecompressionTime += ElapsedTime(&Timer);
  InverseCdf53Old(&QVol, NLevels_);
  //WriteBuffer("out.raw", QVol.Buffer);
  auto Psnr = PSNR(QVolBackup, QVol);
  //fclose(Fp);
  //fclose(Fp2);
  printf("psnr = %f\n", Psnr);
  /* write the bit stream */
  printf("zfp size %lld, time: %f %f\n", TotalCompressedSize, Seconds(TotalCompressionTime), Seconds(TotalDecompressionTime));
}

void TestZfp(volume& Vol, int NLevels, metadata Meta, const v3i& TileDims3, f64 Tolerance) {
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
  timer Timer;
  i64 TotalTime = 0;
  i64 TotalCompressedSize = 0;
  i64 TotalUncompressedSize = 0;

  /* encode the data */
  InitWrite(&Bs, Bs.Stream);
  std::vector<bool> SigVec(Prod(TileDims3));
  v3i BlockDims3(4);
  v3i NBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
  array<i16> EMaxes; Init(&EMaxes, Prod(NBlocks3), i16(0));
  array<i8> Ns; Init(&Ns, Prod(NBlocks3), i8(0));
  array<i8> NOs; Init(&NOs, Prod(NBlocks3), i8(0));
  v3i Dims3 = Dims(Vol);
  std::vector<bool> Check(Prod(Dims3), false);
  i64 TotalEMaxSize = 0;
  i64 TotalEMaxCompressedSize = 0;
  int DstCapacity = LZ4_compressBound(sizeof(i16) * Size(EMaxes));
  buffer EMaxComp; AllocBuf(&EMaxComp, DstCapacity);
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      std::fill(SigVec.begin(), SigVec.end(), false);
      Fill(Begin<u64>(TileVolO), End<u64>(TileVolO), 0);
      Fill(Begin<u64>(TileVol), End<u64>(TileVol), 0);
      Fill(Begin<i8>(Ns), End<i8>(Ns), 0);
      Fill(Begin<i8>(NOs), End<i8>(NOs), 0);
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
      FwdNegaBinary(extent(RealDims3), TileVolQ, extent(RealDims3), &TileVolN);
      /* do zfp transform */
      auto Start = chrono::steady_clock::now();
      //for (int B = 0; B < Prod(NBlocks3); ++B) {
      mg_BeginFor3(P, v3i::Zero, NBlocks3, v3i::One) { // through blocks
        int B = Row(NBlocks3, P);
        extent Ext(P * BlockDims3, BlockDims3);
        EMaxes[B] = Quantize(NBitplanes - 1, Ext, TileVol, Ext, &TileVolQ);
        ForwardZfp((i64*)TileVolQ.Buffer.Data + B * Prod(BlockDims3));
        ForwardShuffle((i64*)TileVolQ.Buffer.Data + B * Prod(BlockDims3),
                       (u64*)TileVolN.Buffer.Data + B * Prod(BlockDims3));
      } mg_EndFor3
      for (int I = Size(EMaxes) - 1; I > 0; --I) {
        EMaxes[I] -= EMaxes[I - 1];
        if (EMaxes[I] >= 0) EMaxes[I] = 2 * EMaxes[I];
        else EMaxes[I] = 2 * abs(EMaxes[I]) - 1;
      }
      TotalEMaxCompressedSize +=
        LZ4_compress_default((char*)&EMaxes[0], (char*)EMaxComp.Data,
                             sizeof(i16) * Size(EMaxes), DstCapacity);
      TotalEMaxSize += 11 * Size(EMaxes);
      auto EndTime = chrono::high_resolution_clock::now();
      auto Diff = EndTime - Start;
      auto Value = std::chrono::duration_cast<std::chrono::nanoseconds>(Diff);
      TotalTime += Value.count();
      /* extract bits */
      int EMax = *(MaxElem(Begin(EMaxes), End(EMaxes)));
      u64 Threshold = (Meta.Type == dtype::float32) ? (1 << 31) : (1ull << 63);
      int Bp = NBitplanes - 1;
      while ((Threshold > 0) && (NBitplanes - Bp <= EMax - Exponent(Tolerance) + 1)) { // through bit planes
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          u64* Beg = (u64*)TileVolN.Buffer.Data + B * Prod(BlockDims3);
          Encode(Beg, Bp, 1e9, Ns[B], &Bs);
        }
        Flush(&Bs);
        TotalCompressedSize += Size(Bs) + 1;
        /* decompress here */
        StartTimer(&Timer);
        Start = chrono::steady_clock::now();
        InitRead(&Bs, Bs.Stream);
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          u64* Beg = (u64*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
          Decode(Beg, Bp, 1e9, NOs[B], &Bs);
        }
        TotalUncompressedSize += Size(Bs);
        InitWrite(&Bs, Bs.Stream);
        Threshold /= 2;
        --Bp;
        TotalTime += ElapsedTime(&Timer);
        EndTime = chrono::high_resolution_clock::now();
        Diff = EndTime - Start;
        Value = std::chrono::duration_cast<std::chrono::nanoseconds>(Diff);
        TotalTime += Value.count();
        ResetTimer(&Timer);
      } // end bit plane loop
    } mg_EndFor3 // end tile loop
  } // end subband loop
  /* write the bit stream */
  Flush(&Bs);
  printf("%lld %lld\n", TotalCompressedSize, TotalUncompressedSize);
  printf("%lld %lld\n", TotalEMaxSize / 8, TotalEMaxCompressedSize);
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

  /* encode the data */
  InitWrite(&Bs, Bs.Stream);
  std::vector<bool> SigVec(Prod(TileDims3));
  array<u16> OriginalPos; Init(&OriginalPos, Prod(TileDims3));
  //Roaring R1;
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      Fill(Begin<u64>(TileVolO), End<u64>(TileVolO), 0);
      Fill(Begin(OriginalPos), End(OriginalPos), 0);
      v3i TileFrom3 = SbFrom3 + TileDims3 * T;
      v3i RealDims3 = Min(SbFrom3 + SbDims3 - TileFrom3, TileDims3);
      int NSamples = Prod(RealDims3);
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
      int Pos = 0, Pos2 = 0;
      while ((Threshold > 0) && (NBitplanes - Bp <= EMax - Exponent(Tolerance) + 1)) { // through bit planes
        // for all significant values
        for (int I = 0; I < Pos; ++I) {
          u64& Val = TileVolN.At<u64>(I);
          Write(&Bs, BitSet(Val, Bp));
        }
        // for all insignificant values
        for (int I = Pos; I < NSamples; ++I) {
          u64& Val = TileVolN.At<u64>(I);
          bool Significant = BitSet(Val, Bp);
          if (Significant) {
            u64& Temp = TileVolN.At<u64>(Pos++);
            Swap(&Temp, &Val);
          }
          Write(&Bs, Significant);
        }

        /* compress the significant map */
        Flush(&Bs);
        TotalUncompressedSize += Size(Bs);
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
        // for all significant values
        for (int I = 0; I < Pos2; ++I) {
          if (Read(&Bs)) {
            u64& Val = TileVolO.At<u64>(I);
            Val |= 1ull << Bp;
          }
        }
        // for all non-significant values
        for (int I = Pos2; I < NSamples; ++I) {
          if (Read(&Bs)) { // significant
            u64& Val = TileVolO.At<u64>(Pos2);
            Val |= 1ull << Bp;
            OriginalPos[Pos2++] = (u16)I;
          }
        }
        InitWrite(&Bs, Bs.Stream);
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
  printf("%lld %lld\n", TotalUncompressedSize, TotalSize);
  //Err = WriteBuffer(OutputFile, buffer(Bs.Stream.Data, Size(Bs)));
  //mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  //Err = WriteBuffer("part2.raw", buffer(Bs2.Stream.Data, Size(Bs2)));
  //mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

int main(int Argc, const char** Argv) {
  auto& A = Perm2<8>;
  for (int I = 0; I < Size(A); ++I) {
    int X = A[I] % 8, Y = A[I] / 8;
    printf("%d %d\n", X, Y);
  }
  //TestJp2k();
  //TestZfp2();
  /* Read data */
  //cstr InputFile, OutputFile;
  //mg_AbortIf(!OptVal(Argc, Argv, "--input", &InputFile), "Provide --input");
  //mg_AbortIf(!OptVal(Argc, Argv, "--output", &OutputFile), "Provide --output");
  //metadata Meta;
  //error Err = ParseMeta(InputFile, &Meta);
  //mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  //mg_AbortIf(Prod<i64>(Meta.Dims) > traits<i32>::Max, "Data dimensions too big");
  //mg_AbortIf(Meta.Type != dtype::float32 && Meta.Type != dtype::float64,
  //           "Data type not supported");
  //int NLevels;
  //mg_AbortIf(!OptVal(Argc, Argv, "--num_levels", &NLevels), "Provide --num_levels");
  //v3i TileDims3;
  //mg_AbortIf(!OptVal(Argc, Argv, "--tile_dims", &TileDims3), "Provide --tile_dims");
  //double Tolerance = 0;
  //mg_AbortIf(!OptVal(Argc, Argv, "--tolerance", &Tolerance), "Provide --tolerance");
  //volume Vol;
  //Err = ReadVolume(InputFile, Meta.Dims, Meta.Type, &Vol);
  //mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  //volume VolCopy;
  //Clone(Vol, &VolCopy);
  //TestZfp(Vol, NLevels, Meta, TileDims3, Tolerance);
  //TestLz4(VolCopy, NLevels, Meta, TileDims3, Tolerance);
}

