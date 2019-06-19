// Testing run-length encoding of wavelet coefficients

#include "mg_array.h"
#include "mg_args.h"
#include "mg_bitops.h"
#include "mg_bitstream.h"
#include "mg_common.h"
#include "mg_io.h"
#include "mg_signal_processing.h"
#include "mg_volume.h"
#include "mg_wavelet.h"
#include "mg_all.cpp"

using namespace mg;

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

  /* perform wavelet transform and allocate necessary buffers */
  ForwardCdf53Old(&Vol, NLevels);
  array<extent> Sbands; BuildSubbands(Meta.Dims, NLevels, &Sbands);
  buffer Buf; AllocBuf(&Buf, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVol(Buf, TileDims3, Vol.Type);
  buffer BufQ; AllocBuf(&BufQ, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolQ(BufQ, TileDims3, IntType(Vol.Type)); // storing quantized values
  buffer BufN; AllocBuf(&BufN, SizeOf(Meta.Type) * Prod(TileDims3));
  volume TileVolN(BufN, TileDims3, IntType(UnsignedType(TileVolQ.Type))); // negabinary
  buffer BsBuf; AllocBuf(&BsBuf, Vol.Buffer.Bytes);
  bitstream Bs; InitWrite(&Bs, BsBuf);
  buffer BsBuf2; AllocBuf(&BsBuf2, Vol.Buffer.Bytes);
  bitstream Bs2; InitWrite(&Bs2, BsBuf2);

  /* encode the data */
  for (int Sb = 0; Sb < Size(Sbands); ++Sb) { // through subbands
    v3i From3 = From(Sbands[Sb]);
    v3i Dims3 = Dims(Sbands[Sb]);
    v3i NTiles3 = (Dims3 + TileDims3 - 1) / TileDims3;
    v3i T; // tile counter
    mg_BeginFor3(T, v3i::Zero, NTiles3, v3i::One) { // through tiles
      v3i TileFrom3 = From3 + TileDims3 * T;
      v3i RealDims3 = Min(From3 + Dims3 - TileFrom3, TileDims3);
      for (u32 Idx = 0; Idx < Prod<u32>(TileDims3); ++Idx) { // through tile samples
        v3i P(DecodeMorton3X(Idx), DecodeMorton3Y(Idx), DecodeMorton3Z(Idx));
        if (!(P < RealDims3)) // outside the domain
          continue;
        if (Meta.Type == dtype::float32)
          TileVol.At<f32>(P) = Vol.At<f32>(P + TileFrom3);
        else if (Meta.Type == dtype::float64)
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
          if (Meta.Type == dtype::float32) {
            u32 Val = TileVolN.At<u32>(I);
            int AlreadySig = (Val >> 1) >= Threshold;
            if (!AlreadySig)
              Write(&Bs, BitSet(Val, Bp));
            else
              Write(&Bs2, BitSet(Val, Bp));
          } else if (Meta.Type == dtype::float64) {
            u64 Val = TileVolN.At<u64>(I);
            int AlreadySig = (Val >> 1) >= Threshold;
            if (!AlreadySig)
              Write(&Bs, BitSet(Val, Bp));
            else
              Write(&Bs2, BitSet(Val, Bp));
          }
        } // end sample loop
        Threshold /= 2;
        --Bp;
      } // end bit plane loop
    } mg_EndFor3 // end tile loop
  } // end subband loop
  /* write the bit stream */
  Flush(&Bs);
  Flush(&Bs2);
  Err = WriteBuffer(OutputFile, buffer(Bs.Stream.Data, Size(Bs)));
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
  Err = WriteBuffer("part2.raw", buffer(Bs2.Stream.Data, Size(Bs2)));
  mg_AbortIf(ErrorExists(Err), "%s", ToString(Err));
}

