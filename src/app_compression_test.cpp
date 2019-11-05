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
//#include <openjp3d/openjpeg.h>

using namespace mg;
namespace chrono = std::chrono;

cstr DataFile_ = "D:/Datasets/3D/Miranda/MIRANDA-DIFFUSIVITY-[384-384-256]-Float64.raw";
int Prec_ = 28;
int NLevels_ = 0; // TODO: check if jpeg's nlevels is the same as our nlevels
int CBlock_ = 32; // size of the code block
v3i InputDims_(384, 384, 256);
dtype InputType_(dtype::float32);
int NBitplanesDecode_[] = { 4, 8, 16, 32 };
i64 TotalDecompressionTimeTzcntAvx2_[4] = {}; // tzcnt + avx2
i64 TotalDecompressionTimeZfp_[4] = {};
i64 TotalDecompressionTimeTzcnt_[4] = {}; // tzcnt
i64 TotalDecompressionTimeAvx2_[4] = {}; // tzcnt

//void TestJp2k() {
//  /* read the data from disk */
//  volume Vol;
//  ReadVolume(DataFile_, InputDims_, InputType_, &Vol);
//  volume QVol(Dims(Vol), dtype::int32);
//  v3i Dims3 = Dims(QVol);
//  Quantize(Prec_, Vol, &QVol);
//  //Clone(Vol ,&QVol);
//  opj_cinfo_t* Compressor = opj_create_compress(CODEC_J3D);
//  opj_cparameters_t Params;
//  opj_set_default_encoder_parameters(&Params);
//  Params.numresolution[2] = Params.numresolution[1] = Params.numresolution[0] = NLevels_ + 1;
//  Params.cblock_init[0] =  Params.cblock_init[1] = Params.cblock_init[2] = CBlock_;
//  Params.tile_size_on = true;
//  Params.cp_tdx = Dims3.X; Params.cp_tdy = Dims3.Y; Params.cp_tdz = Dims3.Z;
//  Params.prog_order = LRCP;
//  Params.encoding_format = ENCOD_3EB;
//  Params.transform_format = TRF_3D_DWT;
//  // MAYBE: precint
//  //Params.prct_init[3]
//  //parameters->res_spec = res_spec;
//  //parameters->csty |= 0x01;
//  // TODO: set subsampling to 1 or 0
//  if (Params.encoding_format == ENCOD_3EB)
//    Params.mode |= (1 << 6);
//
//  if (Params.tcp_numlayers == 0) {
//    Params.tcp_rates[0] = 0.0;
//    Params.tcp_numlayers++;
//    Params.cp_disto_alloc = 1;
//  }
//  opj_volume_cmptparm_t VolParams;
//  VolParams.dx = VolParams.dy = VolParams.dz = 1; // subsampling
//  VolParams.w = Dims3.X; VolParams.h = Dims3.Y; VolParams.l = Dims3.Z;
//  VolParams.x0 = VolParams.y0 = VolParams.z0 = 0;
//  VolParams.prec = VolParams.bpp = Prec_;
//  VolParams.sgnd = 1;
//  VolParams.dcoffset = 0;
//  VolParams.bigendian = 0;
//  opj_volume_t* Volume = opj_volume_create(1, &VolParams, CLRSPC_GRAY);
//  buffer CompBuf((byte*)Volume->comps[0].data, QVol.Buffer.Bytes);
//  MemCopy(QVol.Buffer, &CompBuf);
//  //Volume->comps[0].data = (int*)QVol.Buffer.Data;
//  Volume->numcomps = 1;
//  Volume->comps[0].bpp = Prec_;
//  Volume->x0 = Volume->y0 = Volume->z0 = 0;
//  Volume->x1 = VolParams.w;
//  Volume->y1 = VolParams.h;
//  Volume->z1 = VolParams.l;
//  opj_setup_encoder(Compressor, &Params, Volume);
//  opj_cio_t* Stream = opj_cio_open((opj_common_ptr)Compressor, nullptr, 0); // for writing
//  opj_encode(Compressor, Stream, Volume, nullptr);
//  opj_volume_destroy(Volume);
//  /* decode */
//  auto CompressedBuf = Stream->buffer;
//  auto Length = cio_tell(Stream);
//  //cio_seek(Stream, 0);
//  opj_dinfo_t* Decompressor = opj_create_decompress(CODEC_J3D);
//  opj_dparameters_t DParams;
//  opj_set_default_decoder_parameters(&DParams); // TODO
//  DParams.decod_format = J3D_CFMT;
//  opj_setup_decoder(Decompressor, &DParams);
//  opj_cio_t* DStream = opj_cio_open((opj_common_ptr)Decompressor, CompressedBuf, Length);
//  opj_volume_t* OutVol = opj_decode(Decompressor, DStream);
//  //FILE* Fp = fopen("decompressed.raw", "wb");
//  //fwrite(OutVol->comps[0].data, Prod(Dims3) * sizeof(int), 1, Fp);
//  //fclose(Fp);
//  //OutVol->comps[0]->data;
//  auto Psnr = PSNR(QVol, volume(OutVol->comps[0].data, Dims(QVol)));
//  printf("psnr = %f\n", Psnr);
//  DeallocBuf(&QVol.Buffer);
//  opj_cio_close(Stream);
//  opj_cio_close(DStream);
//  opj_volume_destroy(OutVol);
//  opj_destroy_compress(Compressor);
//  opj_destroy_decompress(Decompressor);
//}

static void error_callback(const char* msg, void* client_data)
{
  (void)client_data;
  fprintf(stdout, "[ERROR] %s", msg);
}
/**
sample warning debug callback expecting no client object
*/
static void warning_callback(const char* msg, void* client_data)
{
  (void)client_data;
  fprintf(stdout, "[WARNING] %s", msg);
}
/**
sample debug callback expecting no client object
*/
static void info_callback(const char* msg, void* client_data)
{
  (void)client_data;
  fprintf(stdout, "[INFO] %s", msg);
}

void TestZfpNewDecoderNew() {
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
  buffer BufO; /*AllocBuf(&BufO, SizeOf(TileVolN.Type) * Prod(TileDims3));*/
  BufO.Data = (byte*)_mm_malloc(SizeOf(TileVolN.Type) * Prod(TileDims3), 32);
  BufO.Bytes = SizeOf(TileVolN.Type) * Prod(TileDims3);
  volume TileVolO(BufO, TileDims3, TileVolN.Type); // decompression output
  buffer BsBuf; AllocBuf(&BsBuf, Vol.Buffer.Bytes);
  bitstream Bs; InitWrite(&Bs, BsBuf);
  timer Timer;
  /* -------- encode the data --------- */
  StartTimer(&Timer);
  v3i BlockDims3(4);
  v3i NBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
  array<i8> Ns; Init(&Ns, Prod(NBlocks3), i8(0));
  array<i8> NOs; Init(&NOs, Prod(NBlocks3), i8(0));
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i SbFrom3 = From(Sbands[Sb]);
    v3i SbDims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (SbDims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      Fill(Begin<u32>(TileVolN), End<u32>(TileVolN), 0);
      Fill(Begin<u32>(TileVolO), End<u32>(TileVolO), 0);
      Fill(Begin<i8>(Ns), End<i8>(Ns), 0);
      Fill(Begin<i8>(NOs), End<i8>(NOs), 0);
      v3i TileFrom3 = SbFrom3 + TileDims3 * T;
      v3i RealDims3 = Min(SbFrom3 + SbDims3 - TileFrom3, TileDims3);
      v3i RealNBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
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
        ForwardZfp((i32*)TileVolQ.Buffer.Data + S);
        ForwardShuffle((i32*)TileVolQ.Buffer.Data + S, (u32*)TileVolN.Buffer.Data + S);
      } mg_EndFor3 // end sample loop
      int NBitplanes = Prec_ + 1 + 3; //Prec + 1;
      int Bp = NBitplanes - 1;
      InitWrite(&Bs, Bs.Stream);
      while (Bp >= 0) { // through bit planes
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          int BX = B % NBlocks3.X;
          int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
            continue;
          u32* Beg = (u32*)TileVolN.Buffer.Data + B * Prod(BlockDims3);
          Encode<u32>(Beg, Bp, 2e9, Ns[B], &Bs);
        }
        --Bp;
      } // end bit plane loop
      Flush(&Bs);
      /* decode here */
      for (int BB = 0; BB < 4; ++BB) {
        i64 Times[17];
        /* new coder 1 */
        //for (int Z = 0; Z < 17; ++Z) {
        //  i64 LocalDecompressionTime = 0;
        //  InitRead(&Bs, Bs.Stream);
        //  Bp = NBitplanes - 1;
        //  for (int I = 0; I < NBitplanesDecode_[BB]; ++I) {
        //    for (int B = 0; B < Prod(NBlocks3); ++B) {
        //      int BX = B % NBlocks3.X;
        //      int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
        //      if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
        //        continue;
        //      u32* BegOut = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
        //      StartTimer(&Timer);
        //      Decode2<u32>(BegOut, Bp, 2e9, NOs[B], &Bs);
        //      LocalDecompressionTime += ElapsedTime(&Timer);
        //    }
        //    --Bp;
        //  }
        //  Times[Z] = LocalDecompressionTime;
        //}
        //std::sort(Times, Times + 17);
        //TotalDecompressionTimeTzcntAvx2_[BB] += Times[8];
        /* new coder 2 */
        //for (int Z = 0; Z < 17; ++Z) {
        //  i64 LocalDecompressionTime = 0;
        //  InitRead(&Bs, Bs.Stream);
        //  Bp = NBitplanes - 1;
        //  for (int I = 0; I < NBitplanesDecode_[BB]; ++I) {
        //    for (int B = 0; B < Prod(NBlocks3); ++B) {
        //      int BX = B % NBlocks3.X;
        //      int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
        //      if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
        //        continue;
        //      u32* BegOut = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
        //      StartTimer(&Timer);
        //      Decode3<u32>(BegOut, Bp, 2e9, NOs[B], &Bs);
        //      LocalDecompressionTime += ElapsedTime(&Timer);
        //    }
        //    --Bp;
        //  }
        //  Times[Z] = LocalDecompressionTime;
        //}
        //std::sort(Times, Times + 17);
        //TotalDecompressionTimeTzcnt_[BB] += Times[8];
        /* new coder 4 */
        for (int Z = 0; Z < 17; ++Z) {
          i64 LocalDecompressionTime = 0;
          InitRead(&Bs, Bs.Stream);
          Bp = NBitplanes - 1;
          for (int I = 0; I < NBitplanesDecode_[BB]; ++I) {
            for (int B = 0; B < Prod(NBlocks3); ++B) {
              int BX = B % NBlocks3.X;
              int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
              if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
                continue;
              u32* BegOut = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
              StartTimer(&Timer);
              Decode4<u32>(BegOut, Bp, 2e9, NOs[B], &Bs);
              LocalDecompressionTime += ElapsedTime(&Timer);
            }
            --Bp;
          }
          Times[Z] = LocalDecompressionTime;
        }
        std::sort(Times, Times + 17);
        TotalDecompressionTimeAvx2_[BB] += Times[8];
        /* zfp coder */
        for (int Z = 0; Z < 17; ++Z) {
          i64 LocalDecompressionTime = 0;
          InitRead(&Bs, Bs.Stream);
          Bp = NBitplanes - 1;
          for (int I = 0; I < NBitplanesDecode_[BB]; ++I) {
            for (int B = 0; B < Prod(NBlocks3); ++B) {
              int BX = B % NBlocks3.X;
              int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
              if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
                continue;
              u32* BegOut = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
              StartTimer(&Timer);
              Decode<u32>(BegOut, Bp, 2e9, NOs[B], &Bs);
              LocalDecompressionTime += ElapsedTime(&Timer);
            }
            --Bp;
          }
          Times[Z] = LocalDecompressionTime;
        }
        std::sort(Times, Times + 17);
        TotalDecompressionTimeZfp_[BB] += Times[8];
      }
    } mg_EndFor3 // end tile loop
  } // end subband loop
  Flush(&Bs);
  /* write the bit stream */
  printf("avx2 time        : %f %f %f %f\n", Seconds(TotalDecompressionTimeAvx2_[0]), Seconds(TotalDecompressionTimeAvx2_[1]),
                                             Seconds(TotalDecompressionTimeAvx2_[2]), Seconds(TotalDecompressionTimeAvx2_[3]));
  printf("tzcnt time       : %f %f %f %f\n", Seconds(TotalDecompressionTimeTzcnt_[0]), Seconds(TotalDecompressionTimeTzcnt_[1]),
                                             Seconds(TotalDecompressionTimeTzcnt_[2]), Seconds(TotalDecompressionTimeTzcnt_[3]));
  printf("tzcnt + avx2 time: %f %f %f %f\n", Seconds(TotalDecompressionTimeTzcntAvx2_[0]), Seconds(TotalDecompressionTimeTzcntAvx2_[1]),
                                             Seconds(TotalDecompressionTimeTzcntAvx2_[2]), Seconds(TotalDecompressionTimeTzcntAvx2_[3]));
  printf("zfp time         : %f %f %f %f\n", Seconds(TotalDecompressionTimeZfp_[0]), Seconds(TotalDecompressionTimeZfp_[1]),
                                             Seconds(TotalDecompressionTimeZfp_[2]), Seconds(TotalDecompressionTimeZfp_[3]));
}
void TestZfpNewDecoder() {
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
  buffer BufO; /*AllocBuf(&BufO, SizeOf(TileVolN.Type) * Prod(TileDims3));*/
  BufO.Data = (byte*)_mm_malloc(SizeOf(TileVolN.Type) * Prod(TileDims3), 32);
  BufO.Bytes = SizeOf(TileVolN.Type) * Prod(TileDims3);
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
      v3i RealNBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
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
        ForwardZfp((i32*)TileVolQ.Buffer.Data + S);
        ForwardShuffle((i32*)TileVolQ.Buffer.Data + S, (u32*)TileVolN.Buffer.Data + S);
      } mg_EndFor3 // end sample loop
      int NBitplanes = Prec_ + 1 + 3; //Prec + 1;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) { // through bit planes
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          int BX = B % NBlocks3.X;
          int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
            continue;
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

  /* ---------- decode ---------- */
  volume QVolBackup; Clone(QVol, &QVolBackup);
  InverseCdf53Old(&QVolBackup, NLevels_);
  Fill(Begin<i32>(QVol), End<i32>(QVol), 0);
  InitRead(&Bs, Bs.Stream);
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
      v3i RealNBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
      /* decompress here */
      int NBitplanes = Prec_ + 1 + 3;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) {
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          int BX = B % NBlocks3.X;
          int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
            continue;
          u32* Beg = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
          StartTimer(&Timer);
          Decode2<u32>(Beg, Bp, 2e9, NOs[B], &Bs);
          TotalDecompressionTime += ElapsedTime(&Timer);
        }
        --Bp;
      }
      v3i P3;
      mg_BeginFor3(P3, v3i::Zero, NBlocks3, v3i::One) { // through blocks
        int S = Row(NBlocks3, P3) * Prod(BlockDims3);
        v3i D3 = P3 * BlockDims3 + TileFrom3;
        InverseShuffle((u32*)TileVolO.Buffer.Data + S, (i32*)TileVolQ.Buffer.Data + S);
        InverseZfp((i32*)TileVolQ.Buffer.Data + S);
        v3i B3;
        mg_BeginFor3(B3, v3i::Zero, BlockDims3, v3i::One) {
          QVol.At<i32>(D3 + B3) = TileVolQ.At<i32>(S + Row(BlockDims3, B3));
        } mg_EndFor3
      } mg_EndFor3
    } mg_EndFor3
  }
  auto DecompressedSize = Size(Bs);
  printf("decompressed size = %lld\n", DecompressedSize);
  InverseCdf53Old(&QVol, NLevels_);
  auto Psnr = PSNR(QVolBackup, QVol);
  printf("psnr = %f\n", Psnr);
  /* write the bit stream */
  printf("zfp size %lld, time: %f %f\n", TotalCompressedSize, Seconds(TotalCompressionTime), Seconds(TotalDecompressionTime));
}
//void TestJp2k2D() {
//  /* read the data from disk */
//  volume Vol;
//  ReadVolume(DataFile_, InputDims_, InputType_, &Vol);
//  volume QVol(Dims(Vol), dtype::int32);
//  v3i Dims3 = Dims(QVol);
//  Quantize(Prec_, Vol, &QVol);
//  opj_codec_t* Compressor = opj_create_compress(OPJ_CODEC_J2K);
//  opj_set_info_handler(Compressor, info_callback, 00);
//  opj_set_warning_handler(Compressor, warning_callback, 00);
//  opj_set_error_handler(Compressor, error_callback, 00);
//
//  opj_cparameters_t Params;
//  opj_set_default_encoder_parameters(&Params);
//  Params.tcp_mct = 0;
//  Params.cod_format = 0; // J2K
//  Params.numresolution = NLevels_ + 1;
//  Params.cblockw_init =  Params.cblockh_init = CBlock_;
//  Params.tile_size_on = false;
//  Params.cp_tdx = Dims3.X; Params.cp_tdy = Dims3.Y;
//  Params.prog_order = OPJ_LRCP;
//  Params.irreversible = 0; // CDF5/3
//
//  if (Params.tcp_numlayers == 0) { // lossless compression
//    Params.tcp_rates[0] = 0.0;
//    Params.tcp_numlayers++;
//    Params.cp_disto_alloc = 1;
//  }
//  /* set up the input "image" */
//  auto CParams = (opj_image_cmptparm_t*)calloc(1, sizeof(opj_image_cmptparm_t));
//  CParams[0].prec = Prec_;
//  CParams[0].bpp = Prec_;
//  CParams[0].sgnd = 1;
//  CParams[0].dx = 1;
//  CParams[0].dy = 1;
//  CParams[0].w = Dims3.X;
//  CParams[0].h = Dims3.Y;
//  auto Image = opj_image_create(1, &CParams[0], OPJ_CLRSPC_GRAY);
//  buffer CompBuf((byte*)Image->comps[0].data, QVol.Buffer.Bytes);
//  MemCopy(QVol.Buffer, &CompBuf);
//  Image->numcomps = 1;
//  Image->x0 = Image->y0 = 0;
//  Image->x1 = CParams[0].w;
//  Image->y1 = CParams[0].h;
//
//  opj_setup_encoder(Compressor, &Params, Image);
//  auto Stream = opj_stream_create_default_file_stream("jp2k-out.raw", OPJ_FALSE);
//  bool Success = opj_start_compress(Compressor, Image, Stream);
//  opj_encode(Compressor, Stream);
//  opj_end_compress(Compressor, Stream);
//  opj_image_destroy(Image);
//  opj_destroy_codec(Compressor);
//  opj_stream_destroy(Stream);
//
//  /* decode */
//  opj_dparameters_t DParams;
//  memset(&DParams, 0, sizeof(opj_dparameters_t));
//  DParams.decod_format = 0; // J2K_CFMT;
//  DParams.cod_format = 18; // RAWL_DFMT 18 /* LSB / Little Endian */
//  // decode tile
//  u32 CompsIndices = 0;
//  opj_set_default_decoder_parameters(&DParams);
//  auto DStream = opj_stream_create_default_file_stream("jp2k-out.raw", 1);
//  auto Decompressor = opj_create_decompress(OPJ_CODEC_J2K);
//  opj_set_info_handler(Decompressor, info_callback, 00);
//  opj_set_warning_handler(Decompressor, warning_callback, 00);
//  opj_set_error_handler(Decompressor, error_callback, 00);
//  opj_setup_decoder(Decompressor, &DParams);
//  opj_codec_set_threads(Decompressor, 1);
//  opj_image_t* DImage = nullptr;
//  opj_read_header(DStream, Decompressor, &DImage);
//  opj_set_decoded_components(Decompressor, 1, &CompsIndices, OPJ_FALSE);
//  opj_decode(Decompressor, DStream, DImage);
//  opj_end_decompress(Decompressor, DStream);
//
//  /* verify using psnr */
//  volume OutVol; Clone(QVol, &OutVol);
//  buffer DecompBuf((byte*)DImage->comps[0].data, QVol.Buffer.Bytes);
//  FILE* Fp = fopen("decompressed.raw", "wb");
//  fwrite(DImage->comps[0].data, QVol.Buffer.Bytes, 1, Fp);
//  MemCopy(DecompBuf, &OutVol.Buffer);
//  auto Ps = PSNR(QVol, OutVol);
//  printf("psnr = %f\n", Ps);
//
//  opj_image_destroy(DImage);
//  opj_stream_destroy(DStream);
//  opj_destroy_codec(Decompressor);
//}

// TODO: use RealDims3 instead of TileDims3
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
      v3i RealNBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
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
          int BX = B % NBlocks3.X;
          int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
            continue;
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
      v3i RealNBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
      /* decompress here */
      int NBitplanes = 20;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) {
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          int BX = B % NBlocks3.X;
          int BZ = ((B - BX) / NBlocks3.X) / NBlocks3.Y, BY = ((B - BX) / NBlocks3.X) % NBlocks3.Y;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y || BZ >= RealNBlocks3.Z)
            continue;
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

void TestZfp2D() {
  /* read the data from disk */
  v3i TileDims3(CBlock_, CBlock_, 1);
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
  v3i BlockDims3(4, 4, 1);
  v3i NBlocks3 = (TileDims3 + BlockDims3 - 1) / BlockDims3;
  array<i8> Ns; Init(&Ns, Prod(NBlocks3), i8(0));
  //FILE* Fp = fopen("encode.raw", "wb");
  FILE* Fp2 = fopen("encode.txt", "w");
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
      v3i RealNBlocks3 = (RealDims3 + BlockDims3 - 1) / BlockDims3;
      v3i P3;
      // TODO: there is a bug if the tile is less than 32^3
      //mg_Assert(RealDims3 == TileDims3);
      mg_BeginFor3(P3, v3i::Zero, RealNBlocks3, v3i::One) { // through blocks
        int S = Row(NBlocks3, P3) * Prod(BlockDims3);
        v3i D3 = P3 * BlockDims3 + TileFrom3;
        v3i B3;
        mg_BeginFor3(B3, v3i::Zero, BlockDims3, v3i::One) {
          mg_Assert(B3.Z == 0);
          TileVolQ.At<i32>(S + Row(BlockDims3, B3)) = QVol.At<i32>(D3 + B3);
        } mg_EndFor3
        //fwrite((i32*)TileVolQ.Buffer.Data + S, Prod(BlockDims3) * sizeof(i32), 1, Fp);
        ForwardZfp2D<i32, 4>((i32*)TileVolQ.Buffer.Data + S);
        //for (int I = 0; I < 4 * 4; ++I) {
        //  i32 Val = *((i32*)TileVolQ.Buffer.Data + S + I);
        //  if (!(Val >= -(1 << 18) && Val < (1 << 18)))
        //    printf("!!!!!!! overflow !!!!!!!!\n");
        //}
        ForwardShuffle2D<i32, u32, 4>((i32*)TileVolQ.Buffer.Data + S, (u32*)TileVolN.Buffer.Data + S);
        //for (int I = 0; I < 16; ++I)
        //  fprintf(Fp2, "%u\n", *((u32*)TileVolN.Buffer.Data + S + I));
      } mg_EndFor3 // end sample loop
      int NBitplanes = Prec_ + 1 + 2; //Prec + 1;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) { // through bit planes
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          int BX = B % NBlocks3.X, BY = B / NBlocks3.X;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y)
            continue;
          u32* Beg = (u32*)TileVolN.Buffer.Data + B * Prod(BlockDims3);
          Encode<u32, 2, 4>(Beg, Bp, 2e9, Ns[B], &Bs);
        }
        --Bp;
      } // end bit plane loop
    } mg_EndFor3 // end tile loop
  } // end subband loop
  TotalCompressionTime += ElapsedTime(&Timer);
  Flush(&Bs);
  TotalCompressedSize += Size(Bs);
  //return;
  //fclose(Fp);
  fclose(Fp2);

  /* ---------- decode ---------- */
  //Fp = fopen("decode.raw", "wb");
  Fp2 = fopen("decode.txt", "w");
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
      v3i RealNBlocks3 = (RealDims3 + BlockDims3 - 1) / BlockDims3;
      /* decompress here */
      int NBitplanes = Prec_ + 1 + 2;
      int Bp = NBitplanes - 1;
      while (Bp >= 0) {
        for (int B = 0; B < Prod(NBlocks3); ++B) {
          int BX = B % NBlocks3.X, BY = B / NBlocks3.X;
          if (BX >= RealNBlocks3.X || BY >= RealNBlocks3.Y)
            continue;
          u32* Beg = (u32*)TileVolO.Buffer.Data + B * Prod(BlockDims3);
          Decode<u32, 2, 4>(Beg, Bp, 2e9, NOs[B], &Bs);
        }
        --Bp;
      }
      v3i P3;
      mg_BeginFor3(P3, v3i::Zero, RealNBlocks3, v3i::One) { // through blocks
        int S = Row(NBlocks3, P3) * Prod(BlockDims3);
        v3i D3 = P3 * BlockDims3 + TileFrom3;
        mg_Assert(D3.Z == 0);
        //for (int I = 0; I < 16; ++I)
        //  fprintf(Fp2, "%u\n", *((u32*)TileVolO.Buffer.Data + S + I));
        InverseShuffle2D<i32, u32, 4>((u32*)TileVolO.Buffer.Data + S, (i32*)TileVolQ.Buffer.Data + S);
        InverseZfp2D<i32, 4>((i32*)TileVolQ.Buffer.Data + S);
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
  fclose(Fp2);
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

__m256i get_mask3(const uint32_t input) {
  __m256i mask = _mm256_set_epi32(0xffffff7f, 0xffffffbf, 0xffffffdf, 0xffffffef, 0xfffffff7, 0xfffffffb, 0xfffffffd, 0xfffffffe);
  __m256i val = _mm256_set1_epi32(input);
  //const __m256i bit_mask(_mm256_set1_epi64x(0x7fbfdfeff7fbfdfe));
  val = _mm256_or_si256(val, mask);
  val = _mm256_cmpeq_epi32(val, _mm256_set1_epi64x(-1));
  int table[8] ALIGNED(32) = { 1, 2, 3, 4, 5, 6, 7, 8 };
  val = _mm256_maskload_epi32(table, val);
  _mm256_store_si256((__m256i*)table, val);
  return val;
}

#include <random>
#include <limits>

// TODO: use 64-bit integer for NBlocks
void
TestFileFormatWrite() {
  v3i N3(96, 96, 96);
  v3i BlockSize3(32);
  v3i BlockSizePlusOne3(BlockSize3 + 1);
  v3i NBlocks3 = (N3 + BlockSize3 - 1) / BlockSize3;
  int NLevels = 1; // TODO: set this based on the block size
  // TODO: check that the block size is a power of 2
  volume Vol; // TODO: initialize this from a file or take a parameter
  ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[96-96-96]-Float64.raw", N3, dtype::float64, &Vol);
  mg_CleanUp(Dealloc(&Vol)); // TODO: fix this macro to remove the first parameter
  /* perform wavelet transform per block, in Z order */
  int NBlocks = Prod<i32>(NBlocks3);
  u32 LastMorton = EncodeMorton3(NBlocks3.X - 1, NBlocks3.Y - 1, NBlocks3.Z - 1);
  volume VolBlock; Resize(&VolBlock, BlockSizePlusOne3, Vol.Type);
  mg_CleanUp(Dealloc(&VolBlock));
  volume VolBlockBackup; Resize(&VolBlockBackup, Dims(VolBlock), VolBlock.Type);
  mg_CleanUp(Dealloc(&VolBlockBackup));
  for (u32 BMorton = 0; BMorton <= LastMorton; ++BMorton) {
    v3i B3(DecodeMorton3X(BMorton), DecodeMorton3Y(BMorton), DecodeMorton3Z(BMorton));
    if (!(B3 < NBlocks3)) { // if block is outside the domain, skip to the next block
      int B = Lsb(BMorton);
      mg_Assert(B >= 0);
      BMorton = (((BMorton >> (B + 1)) + 1) << (B + 1)) - 1;
      continue;
    }
    /* copy data over to a local buffer and perform wavelet transform */
    extent BlockExt(B3 * BlockSize3, BlockSize3);
    extent BlockExtCrop = Crop(BlockExt, extent(N3));
    extent BlockExtLocal = Relative(BlockExtCrop, BlockExt);
    Copy(BlockExtCrop, Vol, BlockExtLocal, &VolBlock);
    Clone(VolBlock, &VolBlockBackup);
    v3i D3 = Dims(BlockExtLocal); // Dims
    v3i R3 = D3 + IsEven(D3);
    v3i S3(1); // Strides
    stack_array<v3<v3i>, 10> Dims3Stack; mg_Assert(NLevels < Size(Dims3Stack));
    for (int L = 0; L < NLevels; ++L) {
      // TODO: replace f64 with a template param
      FLiftCdf53X<f64>(grid(v3i(0), v3i(D3.X, D3.Y, D3.Z), S3), BlockSize3, lift_option::Normal, &VolBlock);
      FLiftCdf53Y<f64>(grid(v3i(0), v3i(R3.X, D3.Y, D3.Z), S3), BlockSize3, lift_option::Normal, &VolBlock);
      FLiftCdf53Z<f64>(grid(v3i(0), v3i(R3.X, R3.Y, D3.Z), S3), BlockSize3, lift_option::Normal, &VolBlock);
      Dims3Stack[L] = v3(v3i(D3.X, D3.Y, D3.Z), v3i(R3.X, D3.Y, D3.Z), v3i(R3.X, R3.Y, D3.Z));
      D3 = (R3 + 1) / 2;
      R3 = D3 + IsEven(D3);
      S3 = S3 * 2;
    }
    for (int L = NLevels - 1; L >= 0; --L) {
      S3 = S3 / 2;
      ILiftCdf53Z<f64>(grid(v3i(0), Dims3Stack[L].Z, S3), BlockSize3, lift_option::Normal, &VolBlock);
      ILiftCdf53Y<f64>(grid(v3i(0), Dims3Stack[L].Y, S3), BlockSize3, lift_option::Normal, &VolBlock);
      ILiftCdf53X<f64>(grid(v3i(0), Dims3Stack[L].X, S3), BlockSize3, lift_option::Normal, &VolBlock);
    }
    f64 Ps = PSNR(BlockExtLocal, VolBlockBackup, BlockExtLocal, VolBlock);
    printf("psnr = %f\n", Ps);
  }
  // TODO: then test that we can group coefficients across blocks, write to file, and read them back
  // TODO: we should test that we can inverse transform to exactly the same data
  // TODO: we should test that this works for all dims not multiples of 32
}

void
TestWaveletBlock() {
  int N = 256;
  FILE* Fp = fopen("func.txt", "w");
  FILE* Fp2 = fopen("recon.txt", "w");
  FILE* Fp3 = fopen("smooth.txt", "w");
  array<f64> Arr; Init(&Arr, N);
  for (int I = 0; I < Size(Arr); ++I) {
    Arr[I] = cos(f64(I) / 4);
    fprintf(Fp, "%f\n", Arr[I]);
  }
  int NLevels = 1;
  int BlockSize = 32;
  int NBlocks= (N + BlockSize - 1) / BlockSize;
  /* Copy Arr to another array and extrapolate the last position */
  array<f64> Arr2; Init(&Arr2, N + NBlocks);
  for (int B = 0; B < NBlocks; ++B) {
    for (int I = 0; I < BlockSize; ++I) {
      Arr2[B * (BlockSize + 1) + I] = Arr[B * BlockSize + I];
    }
    Arr2[B * (BlockSize + 1) + (BlockSize)] = 2 * Arr[B * BlockSize + BlockSize - 1] - Arr[B * BlockSize + BlockSize - 2];
  }
  for (int B = 0; B < NBlocks; ++B) {
    for (int I = 0; I < NLevels; ++I) {
      FLiftCdf53OldX(&Arr[B * BlockSize], v3i(BlockSize, 1, 1), v3i(I));
    }
  }
  array<extent> Subbands; BuildSubbands(v3i(BlockSize, 1, 1), NLevels, &Subbands);
  extent Ext = Back(Subbands);
  for (int B = 0; B < NBlocks; ++B) {
    for (int X = From(Ext).X; X < To(Ext).X; ++X) {
      Arr[B * BlockSize + X] = 0;
    }
  }
  for (int B = 0; B < NBlocks; ++B) {
    for (int I = NLevels - 1; I >= 0; --I) {
      ILiftCdf53OldX(&Arr[B * BlockSize], v3i(BlockSize, 1, 1), v3i(I));
    }
  }
  for (int I = 0; I < Size(Arr); ++I) {
    fprintf(Fp2, "%f\n", Arr[I]);
  }
  for (int B = 0; B < NBlocks; ++B) {
    for (int I = 0; I < NLevels; ++I) {
      FLiftCdf53OldX(&Arr2[B * (BlockSize + 1)], v3i(BlockSize + 1, 1, 1), v3i(I));
    }
  }
  array<extent> Subbands2; BuildSubbands(v3i(BlockSize + 1, 1, 1), NLevels, &Subbands2);
  Ext = Back(Subbands2);
  for (int B = 0; B < NBlocks; ++B) {
    for (int X = From(Ext).X; X < To(Ext).X; ++X) {
      Arr2[B * (BlockSize + 1) + X] = 0;
    }
  }
  for (int B = 0; B < NBlocks; ++B) {
    for (int I = NLevels - 1; I >= 0; --I) {
      ILiftCdf53OldX(&Arr2[B * (BlockSize + 1)], v3i(BlockSize + 1, 1, 1), v3i(I));
    }
    for (int I = 0; I < BlockSize; ++I) {
      fprintf(Fp3, "%f\n", Arr2[B * (BlockSize + 1) + I]);
    }
  }
  Dealloc(&Arr);
  Dealloc(&Arr2);
  fclose(Fp);
  fclose(Fp2);
  fclose(Fp3);
}

int main(int Argc, const char** Argv) {
  //TestWaveletBlock();
  TestFileFormatWrite();
  return 0;
  /*  */
  // TODO: move this into a standalone app
  // TODO: use memmap to read file
  // TODO: Write a adaptor for volume using memmap
  v3i N3(384, 384, 256);
  int NLevels = 4;
  int BlockSize = 32;
  f64 PsnrThreshold = 16;
  f64 NormThreshold = 0.2;
  wav_basis_norms Wn = GetCdf53Norms(NLevels);
  volume Vol;
  ReadVolume("D:/Datasets/3D/Miranda/MIRANDA-DENSITY-[384-384-256]-Float32.raw", N3, dtype::float32, &Vol);
  mg_CleanUp(4, Dealloc(&Vol));
  volume VolOut; Resize(&VolOut, Dims(Vol), dtype::int16);
  volume DataOut; Resize(&DataOut, Dims(Vol), Vol.Type);
  v3i NBlocks = (N3 + BlockSize - 1) / BlockSize;
  array<extent> Subbands;
  array<grid> SubbandsG;
  BuildSubbands(v3i(BlockSize), NLevels, &Subbands);
  BuildSubbands(v3i(BlockSize), NLevels, &SubbandsG);
  mg_CleanUp(0, Dealloc(&Subbands));
  mg_CleanUp(5, Dealloc(&SubbandsG));
  volume BlockVol; Resize(&BlockVol, v3i(BlockSize), Vol.Type);
  mg_CleanUp(1, Dealloc(&BlockVol));
  volume BackupVol; Resize(&BackupVol, Dims(BlockVol), BlockVol.Type);
  mg_CleanUp(2, Dealloc(&BackupVol));
  volume WavBackupVol; Resize(&WavBackupVol, Dims(BlockVol), BlockVol.Type);
  mg_CleanUp(3, Dealloc(&WavBackupVol));
  i64 CoeffCount = 0;
  // Write the block headers
  FILE* Fp = fopen("blocks.raw", "wb");
  //fwrite(&Vol.Type, sizeof(Vol.Type), 1, Fp); // TODO: here we assume the data type is always float
  fwrite(&NBlocks.X, sizeof(NBlocks.X), 1, Fp); // number of blocks in each dimension
  fwrite(&NBlocks.Y, sizeof(NBlocks.Y), 1, Fp);
  fwrite(&NBlocks.Z, sizeof(NBlocks.Z), 1, Fp);
  fwrite(&BlockSize, sizeof(BlockSize), 1, Fp);
  for (int BZ = 0; BZ < NBlocks.Z; ++BZ) {
    for (int BY = 0; BY < NBlocks.Y; ++BY) {
      for (int BX = 0; BX < NBlocks.X; ++BX) {
        Fill(Begin<f32>(BlockVol), End<f32>(BlockVol), 0.0f);
        extent BlockExt(v3i(BX, BY, BZ) * BlockSize, v3i(BlockSize));
        extent BlockExtCrop = Crop(BlockExt, extent(N3));
        Copy(BlockExtCrop, Vol, Relative(BlockExtCrop, BlockExt), &BlockVol);
        Clone(BlockVol, &BackupVol);
        for (int I = 0; I < NLevels; ++I) {
          FLiftCdf53OldX((f32*)BlockVol.Buffer.Data, v3i(BlockSize), v3i(I));
          FLiftCdf53OldY((f32*)BlockVol.Buffer.Data, v3i(BlockSize), v3i(I));
          FLiftCdf53OldZ((f32*)BlockVol.Buffer.Data, v3i(BlockSize), v3i(I));
        }
        Clone(BlockVol, &WavBackupVol);
        Fill(Begin<f32>(BlockVol), End<f32>(BlockVol), 0.0);
        /* go through each subband, compute the norm of wavelet coefficients x basis norm */
        int LastSb = 0;
        for (int I = 0; I < Size(Subbands); ++I) {
          extent Ext = Subbands[I];
          v3i Lvl3 = SubbandToLevel(3, I);
          f64 Score = 0;
          if (I == 0) {
            Score = Wn.ScalNorms[NLevels - 1];
            Score = Score * Score * Score;
          } else {
            int LMax = Max(Max(Lvl3.X, Lvl3.Y), Lvl3.Z);
            f64 Sx = (Lvl3.X == LMax) ? Wn.WaveNorms[NLevels - LMax] : Wn.ScalNorms[NLevels - LMax];
            f64 Sy = (Lvl3.Y == LMax) ? Wn.WaveNorms[NLevels - LMax] : Wn.ScalNorms[NLevels - LMax];
            f64 Sz = (Lvl3.Z == LMax) ? Wn.WaveNorms[NLevels - LMax] : Wn.ScalNorms[NLevels - LMax];
            Score = Sx * Sy * Sz;
          }
          for (auto It = Begin<f32>(Ext, WavBackupVol), It2 = Begin<f32>(Ext, BlockVol);
                    It != End<f32>(Ext, WavBackupVol); ++It, ++It2)
          {
            f64 FinalScore = Score * fabs(*It);
            if (FinalScore >= NormThreshold) {
              *It2 = *It;
              LastSb = I;
              ++CoeffCount;
            }
          }
        }
        //LastSb = Size(Subbands) - 8;
        v3i Strd3O = Strd(SubbandsG[LastSb]);
        v3i SbLvl3 = SubbandToLevel(3, LastSb, true);
        v3i Denom3 = (1 << SbLvl3);
        if (Denom3.X == 1)
          Denom3.Y = Denom3.Z = 1;
        if (Denom3.Y = 1)
          Denom3.Z = 1;
        v3i Strd3 = Strd3O / Denom3;
        v3i Dims3 = (Dims(BlockVol) + Strd3 - 1) / Strd3;
        printf("dims = %d %d %d\n", Dims3.X, Dims3.Y, Dims3.Z);
        Fill(Begin<i16>(BlockExtCrop, VolOut), End<i16>(BlockExtCrop, VolOut), i16(LastSb));
        //printf("Last sb = %d\n", LastSb);
        // TODO: inverse to LastSb only
        for (int I = NLevels - 1; I >= 0; --I) {
          ILiftCdf53OldZ((f32*)BlockVol.Buffer.Data, v3i(BlockSize), v3i(I));
          ILiftCdf53OldY((f32*)BlockVol.Buffer.Data, v3i(BlockSize), v3i(I));
          ILiftCdf53OldX((f32*)BlockVol.Buffer.Data, v3i(BlockSize), v3i(I));
        }
        Copy(Relative(BlockExtCrop, BlockExt), BlockVol, BlockExtCrop, &DataOut);
        // file format:
        // dx, dy, dz, data for block 1
        // dx, dy, dz, data for block 2
        // ...
        // dx, dy, dz, data for block n
        // TODO: don't need 32 bit for the dims
        fwrite(&Dims3.X, sizeof(Dims3.X), 1, Fp);
        fwrite(&Dims3.Y, sizeof(Dims3.Y), 1, Fp);
        fwrite(&Dims3.Z, sizeof(Dims3.Z), 1, Fp);
        for (int Z = 0; Z < Dims3.Z; ++Z) {
        for (int Y = 0; Y < Dims3.Y; ++Y) {
        for (int X = 0; X < Dims3.X; ++X) {
          fwrite(&BlockVol.At<f32>(v3i(X, Y, Z) * Strd3), sizeof(f32), 1, Fp);
          //fwrite(&BlockVol.At<f32>(v3i(X, Y, Z) * Strd3), sizeof(f32), 1, Fp2);
        }}}
        //f64 Ps = PSNR(BackupVol, BlockVol);
      }
    }
  }
  fclose(Fp);
  //fclose(Fp2);
  WriteBuffer("out.raw", VolOut.Buffer);
  WriteBuffer("dataout.raw", DataOut.Buffer);
  printf("Coeff count = %lld\n", CoeffCount);
  //printf("psnr = %f\n", Ps);
  //WriteBuffer("vol.raw", Vol.Buffer);
  //WriteBuffer("wav.raw", WavBackupVol.Buffer);
  //Dealloc(&Subbands);
  //for (int I = 0; I < 8; ++I) {
  //  v2i L = SubbandToLevel2(I);
  //  printf("Subband to level %d: %d %d\n", I, L.X, L.Y);
  //}
  //get_mask3(0x0A0B0C0D);
  //TestJp2k();
  //TestZfp2D();
  //u64 Val = 1729382256910270464ull;
  //TestZfpNewDecoderNew();
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

