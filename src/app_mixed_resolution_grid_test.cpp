#include <random>
#include <limits>
#include "mg_all.cpp"
#include <stdio.h>

using namespace mg;

// TODO: use 64-bit integer for NBlocks
void
TestFileFormatWrite() {
  v3i N3(96, 96, 96);
  v3i BlockSize3(32);
  v3i BlockSizePlusOne3(BlockSize3 + 1);
  v3i NBlocks3 = (N3 + BlockSize3 - 1) / BlockSize3;
  int NLevels = 4; // TODO: set this based on the block size
  // TODO: check that the block size is a power of 2
  volume Vol; // TODO: initialize this from a file or take a parameter
  ReadVolume("D:/Datasets/3D/Small/MIRANDA-DENSITY-[96-96-96]-Float64.raw", N3, dtype::float64, &Vol);
  mg_CleanUp(Dealloc(&Vol)); // TODO: fix this macro to remove the first parameter
  /* perform wavelet transform per block, in Z order */
  u32 LastMorton = EncodeMorton3(NBlocks3.X - 1, NBlocks3.Y - 1, NBlocks3.Z - 1);
  volume VolBlock; Resize(&VolBlock, BlockSizePlusOne3, Vol.Type);
  mg_CleanUp(Dealloc(&VolBlock));
  volume VolBlockBackup; Resize(&VolBlockBackup, Dims(VolBlock), VolBlock.Type);
  mg_CleanUp(Dealloc(&VolBlockBackup));
  /* loop through the blocks in morton order */
  FILE* Fp = fopen("file.wz0", "wb"); // version 0, everything in one file, no interleaving of bit planes
  mg_CleanUp(if (Fp) fclose(Fp));
  fwrite(&N3, sizeof(N3), 1, Fp); // overall dimensions
  fwrite(&Vol.Type, sizeof(Vol.Type), 1, Fp); // data type
  fwrite(&BlockSize3, sizeof(BlockSize3), 1, Fp); // block size
  fwrite(&NLevels, sizeof(NLevels), 1, Fp); // number of levels
  bool DoExtrapolation = true;
  fwrite(&DoExtrapolation, sizeof(DoExtrapolation), 1, Fp); // whether the wavelet transform is done with extrapolation
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
    /* uncomment the below to test the inverse transform */
    // for (int L = NLevels - 1; L >= 0; --L) {
    //   S3 = S3 / 2;
    //   ILiftCdf53Z<f64>(grid(v3i(0), Dims3Stack[L].Z, S3), BlockSize3, lift_option::Normal, &VolBlock);
    //   ILiftCdf53Y<f64>(grid(v3i(0), Dims3Stack[L].Y, S3), BlockSize3, lift_option::Normal, &VolBlock);
    //   ILiftCdf53X<f64>(grid(v3i(0), Dims3Stack[L].X, S3), BlockSize3, lift_option::Normal, &VolBlock);
    // }
    // f64 Ps = PSNR(BlockExtLocal, VolBlockBackup, BlockExtLocal, VolBlock);
    // printf("psnr = %f\n", Ps);
    /* quantize the coefficients, and write a common exponent for all  */
    WriteVolume(Fp, VolBlock, extent(VolBlock));
  } // end block loop
  fclose(Fp);
  // TODO: then test that we can group coefficients across blocks, write to file, and read them back
  // TODO: we should test that we can inverse transform to exactly the same data
  // TODO: we should test that this works for all dims not multiples of 32
  // TODO: test the zfp compression to see how much more data we are saving compared to no extrapolation
  // TODO: measure the seek time and read time on my hard drives
  // TODO: fit a curve between the read times and the number of bytes read (to extrapolate)
  // TODO: write a point sampling routine for the mixed-resolution grid and compare extrapolation vs non-extrapolation
  //       basically we want to show that extrapolation creates a "smooth" field
}

/* Downsample a 1D signal in three different ways: globally, by block, and by
block with extrapolation. The signal is then upsampled (using wavelets) to the
finest resolution to be compared with the original signal. The original signal
can be either a cosine wave, or real data read from a file. */
void
TestBlockBasedUpsampling() {
  int N = 384;
  FILE* Fp = fopen("func.txt", "w");
  FILE* Fp2 = fopen("recon.txt", "w");
  FILE* Fp3 = fopen("smooth.txt", "w");
  FILE* Fp4 = fopen("full.txt", "w");
  array<f64> Arr; Init(&Arr, N);
  ReadFile("D:/Datasets/1D/MIRANDA-PRESSURE-[384]-Float64.raw", &Arr.Buffer);
  for (int I = 0; I < Size(Arr); ++I) {
    // Arr[I] = cos(f64(I) / 4);
    fprintf(Fp, "%f\n", Arr[I]);
  }
  int NLevels = 4;
  int BlockSize = 32;
  int K = 3;
  int NBlocks= (N + BlockSize - 1) / BlockSize;
  /* Copy Arr to another array and extrapolate the last position */
  array<f64> Arr2; Init(&Arr2, N + NBlocks);
  /* copy Arr to another array */
  array<f64> Arr3; Clone(Arr, &Arr3);
  for (int B = 0; B < NBlocks; ++B) {
    for (int I = 0; I < BlockSize; ++I)
      Arr2[B * (BlockSize + 1) + I] = Arr[B * BlockSize + I];
    Arr2[B * (BlockSize + 1) + (BlockSize)] = 2 * Arr[B * BlockSize + BlockSize - 1] - Arr[B * BlockSize + BlockSize - 2];
  }
  array<extent> Subbands; BuildSubbands(v3i(BlockSize, 1, 1), NLevels, &Subbands);
  /* for each block */
  for (int B = 0; B < NBlocks; ++B) {
    /* do forward transform */
    v3i M(BlockSize, 1, 1);
    for (int I = 0; I < NLevels; ++I)
      // FLiftCdf53ConstX(&Arr[B * BlockSize], M, v3i(I));
      FLiftCdf53OldX(&Arr[B * BlockSize], M, v3i(I));
    /* set the last K subbands to 0 */
    for (int J = 0; J < K; ++J) {
      const extent& Ext = Subbands[Size(Subbands) - J - 1];
      for (int X = From(Ext).X; X < To(Ext).X; ++X)
        Arr[B * BlockSize + X] = 0;
    }
    /* inverse transform to the finest level */
    for (int I = NLevels - 1; I >= 0; --I)
      //ILiftCdf53ConstX(&Arr[B * BlockSize], M, v3i(I));
      ILiftCdf53OldX(&Arr[B * BlockSize], M, v3i(I));
    for (int I = 0; I < BlockSize; ++I)
      fprintf(Fp2, "%f\n", Arr[B * (BlockSize) + I]);
  }

  /* do the same, but with extrapolation */
  array<extent> SubbandsExt; BuildSubbands(v3i(BlockSize + 1, 1, 1), NLevels, &SubbandsExt);
  for (int B = 0; B < NBlocks; ++B) {
    /* do forward transform */
    v3i M(BlockSize + 1, 1, 1);
    for (int I = 0; I < NLevels; ++I)
      FLiftCdf53OldX(&Arr2[B * (BlockSize + 1)], M, v3i(I));
    /* set the last K subbands to 0 */
    for (int J = 0; J < K; ++J) {
      const extent& Ext = SubbandsExt[Size(SubbandsExt) - J - 1];
      for (int X = From(Ext).X; X < To(Ext).X; ++X)
        Arr2[B * (BlockSize + 1) + X] = 0;
    }
    for (int I = NLevels - 1; I >= 0; --I)
      ILiftCdf53OldX(&Arr2[B * (BlockSize + 1)], M, v3i(I));
    for (int I = 0; I < BlockSize; ++I)
      fprintf(Fp3, "%f\n", Arr2[B * (BlockSize + 1) + I]);
  }

  /* do the same but globally */
  /* do forward transform */
  for (int I = 0; I < NLevels; ++I)
    FLiftCdf53OldX(&Arr3[0], v3i(N, 1, 1), v3i(I));
  array<extent> SubbandsFull; BuildSubbands(v3i(N, 1, 1), NLevels, &SubbandsFull);
  /* set the last K subbands to 0 */
  for (int J = 0; J < K; ++J) {
    const extent& Ext = SubbandsFull[Size(SubbandsFull) - J - 1];
    for (int X = From(Ext).X; X < To(Ext).X; ++X)
      Arr3[X] = 0;
  }
  for (int I = NLevels - 1; I >= 0; --I)
    ILiftCdf53OldX(&Arr3[0], v3i(N, 1, 1), v3i(I));
  for (int I = 0; I < N; ++I)
    fprintf(Fp4, "%f\n", Arr3[I]);

  /* clean up */
  Dealloc(&Arr);
  Dealloc(&Arr2);
  Dealloc(&Arr3);
  fclose(Fp);
  fclose(Fp2);
  fclose(Fp3);
  fclose(Fp4);
}

mg_T(t)
struct MixedResGrids {
  array<i64> Indices; // [block idx] -> sample idx
  array<v3i> BlockRes3; // block resolutions
  array<t> Samples;
  v3i BlockSize3; // logical block size
  v3i N3; // global dimensions
  dtype DType;

  /* Evaluate the function at a point */
  mg_T(t) t
  At(const v3d& Pos3) const {
    mg_Assert(Pos3 < v3d(N3));
    // TODO: find the primary block
    v3i NBlocks3 = (N3 + BlockSize3 - 1) / BlockSize3;
    v3i BlockIdx3 = Pos3 / v3d(BlockSize3);
    i64 BlockIdx = Row(NBlocks3, BlockIdx3);
    v3d LocalPos3 = Pos3 - (BlockIdx3 * BlockSize3);
    v3i LocalIdx3 = LocalPos3;
    mg_Assert(LocalIdx3 < BlockRes3[BlockIdx]);
    if (NumDims(N3) == 2) {
      t V00 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(0, 0, 0))];
      t V10 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(1, 0, 0))];
      t V01 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(0, 1, 0))];
      t V11 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(1, 1, 0))];
      return BiLerp(V00, V10, V01, V11, (LocalPos3 - LocalIdx3).XY);
    } else if (NumDims(N3) == 3) {
      t V000 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(0, 0, 0))];
      t V100 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(1, 0, 0))];
      t V010 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(0, 1, 0))];
      t V110 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(1, 1, 0))];
      t V001 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(0, 0, 1))];
      t V101 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(1, 0, 1))];
      t V011 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(0, 1, 1))];
      t V111 = Samples[BlockIdx + Row(BlockRes3[BlockIdx], LocalIdx3 + v3i(1, 1, 1))];
      return TriLerp(V000, V100, V010, V110, V001, V101, V011, V111, LocalPos3 - LocalIdx3);
    } else {
      mg_Assert(false);
    }
    return 0;
  }
};

mg_T(t) void
ReadMultiGrid(cstr* FileName, MixedResGrids<t>* Grid) {
  FILE* Fp = fopen(FileName, "rb");
  fread(&Grid->N3, sizeof(Grid->N3), 1, Fp);
  fread(&Grid->DType, sizeof(Grid->DType), 1, Fp);
  fread(&Grid->BlockSize3, sizeof(Grid->BlockSize3), 1, Fp);
  v3i NBlocks3 = (N3 + BlockSize3 - 1) / BlockSize3;
  Resize(&Indices, Prod<i64>(NBlocks3));
  Resize(&BlockRes3, Size(Indices));
  for (i64 I = 0; I < Size(Indices); ++I) {
    fread(&Grid->BlockRes[I], sizeof(Grid->BlockRes[I]), 1, Fp);
    Indices[I] = Size(Grid->Samples);
    for (int J = 0; J < Prod(Grid->BlockRes[I]); ++J) {
      t S; fread(&S, sizeof(S), 1, Fp);
      PushBack(&Samples, S);
    }
  }
  fclose(Fp);
}

/* Generate the blocks at different resolutions (the set of wavelet coefficients
is determined by the absolute values and their levels). The blocks are then
written to a file to be read later. */
mg_T(t) void
TestBlockGeneration(cstr RawFile, dtype DType) {
  if constexpr(is_same_type<t, f64>::Value)
    mg_Assert(DType == dtype::float64);
  if constexpr(is_same_type<t, f32>::Value)
    mg_Assert(DType == dtype::float32);
  // TODO: use memmap to read file
  // TODO: Write a adaptor for volume using memmap
  v3i N3(384, 384, 256);
  int NLevels = 4;
  v3i BlockSize3(32, 32, 32);
  f64 NormThreshold = 0.2;
  wav_basis_norms Wn = GetCdf53Norms(NLevels);
  mg_RAII(volume, Vol, ReadVolume(RawFile, N3, DType, &Vol));
  mg_RAII(volume, VolRefinement, Resize(&VolRefinement, Dims(Vol), dtype::int8));
  mg_RAII(volume, DataOut, Resize(&DataOut, Dims(Vol), Vol.Type));
  v3i NBlocks = (N3 + BlockSize3 - 1) / BlockSize3;
  mg_RAII(array<extent>, Subbands, BuildSubbands(BlockSize3, NLevels, &Subbands));
  mg_RAII(array<grid>, SubbandsG, BuildSubbands(BlockSize3, NLevels, &SubbandsG));
  mg_RAII(volume, BlockVol, Resize(&BlockVol, BlockSize3, Vol.Type));
  mg_RAII(volume, BackupVol, Resize(&BackupVol, Dims(BlockVol), BlockVol.Type));
  mg_RAII(volume, WavBackupVol, Resize(&WavBackupVol, Dims(BlockVol), BlockVol.Type));
  i64 CoeffCount = 0;
  // Write the block headers
  mg_RAII(FILE*, Fp, Fp = fopen("blocks.raw", "wb"), if (Fp) fclose(Fp));
  fwrite(&N3, sizeof(N3), 1, Fp); // write the global dimensions
  fwrite(&Vol.Type, sizeof(Vol.Type), 1, Fp); // write data type
  fwrite(&BlockSize3, sizeof(BlockSize3), 1, Fp); // write block size
  v3i B3;
  mg_BeginFor3(B3, v3i::Zero, NBlocks, v3i::One) {
    Fill(Begin<t>(BlockVol), End<t>(BlockVol), t(0));
    extent BlockExt(B3 * BlockSize3, BlockSize3);
    extent BlockExtCrop = Crop(BlockExt, extent(N3));
    extent BlockExtLocal = Relative(BlockExtCrop, BlockExt);
    Copy(BlockExtCrop, Vol, BlockExtLocal, &BlockVol);
    Clone(BlockVol, &BackupVol);
    /* forward wavelet transform */
    v3i D3 = Dims(BlockExtLocal); // Dims
    v3i R3 = D3 + IsEven(D3);
    v3i S3(1); // Strides
    stack_array<v3<v3i>, 10> Dims3Stack; mg_Assert(NLevels < Size(Dims3Stack));
    for (int I = 0; I < NLevels; ++I) {
      FLiftCdf53X<f64>(grid(v3i(0), v3i(D3.X, D3.Y, D3.Z), S3), BlockSize3, lift_option::Normal, &VolBlock);
      FLiftCdf53Y<f64>(grid(v3i(0), v3i(R3.X, D3.Y, D3.Z), S3), BlockSize3, lift_option::Normal, &VolBlock);
      FLiftCdf53Z<f64>(grid(v3i(0), v3i(R3.X, R3.Y, D3.Z), S3), BlockSize3, lift_option::Normal, &VolBlock);
      Dims3Stack[L] = v3(v3i(D3.X, D3.Y, D3.Z), v3i(R3.X, D3.Y, D3.Z), v3i(R3.X, R3.Y, D3.Z));
      D3 = (R3 + 1) / 2;
      R3 = D3 + IsEven(D3);
      S3 = S3 * 2;
    }
    Clone(BlockVol, &WavBackupVol);
    Fill(Begin<t>(BlockVol), End<t>(BlockVol), t(0));
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
      /* copy the relevant coefficients over */
      for (auto It  = Begin<t>(Ext, WavBackupVol), It2 = Begin<t>(Ext, BlockVol);
                It !=   End<t>(Ext, WavBackupVol); ++It, ++It2)
      {
        f64 FinalScore = Score * fabs(*It);
        if (FinalScore >= NormThreshold) {
          *It2 = *It;
          LastSb = I;
          ++CoeffCount;
        }
      }
    }
    /* compute the relevant resolution for the block (stored in Dims3) */
    v3i Strd3O = Strd(SubbandsG[LastSb]);
    v3i SbLvl3 = SubbandToLevel(NumDims(N3), LastSb, true);
    v3i Denom3 = (1 << SbLvl3);
    if (Denom3.X == 1) Denom3.Y = 1;
    if (Denom3.Y == 1) Denom3.Z = 1;
    v3i Strd3 = Strd3O / Denom3;
    v3i Dims3 = (Dims(BlockVol) + Strd3 - 1) / Strd3;
    // TODO: check that this code works for boundary blocks with dims < BlockSize
    printf("dims = %d %d %d\n", Dims3.X, Dims3.Y, Dims3.Z);
    Fill(Begin<i8>(BlockExtCrop, VolRefinement), End<i8>(BlockExtCrop, VolRefinement), i8(LastSb));
    //printf("Last sb = %d\n", LastSb);
    // TODO: inverse to LastSb only
    /* inverse transform to reconstruct the block */
    for (int I = NLevels - 1; I >= 0; --I) {
      S3 = S3 / 2;
      ILiftCdf53Z<f64>(grid(v3i(0), Dims3Stack[L].Z, S3), BlockSize3, lift_option::Normal, &BlockVol);
      ILiftCdf53Y<f64>(grid(v3i(0), Dims3Stack[L].Y, S3), BlockSize3, lift_option::Normal, &BlockVol);
      ILiftCdf53X<f64>(grid(v3i(0), Dims3Stack[L].X, S3), BlockSize3, lift_option::Normal, &BlockVol);
    }
    Copy(Relative(BlockExtCrop, BlockExt), BlockVol, BlockExtCrop, &DataOut);
    // file format:
    // dx, dy, dz, data for block 1
    // dx, dy, dz, data for block 2
    // ...
    // dx, dy, dz, data for block n
    // TODO: don't need 32 bit for the dims
    /* dump the raw values at the computed resolution (Dims3) */
    fwrite(&Dims3, sizeof(Dims3), 1, Fp);
    for (int Z = 0; Z < Dims3.Z; ++Z) {
    for (int Y = 0; Y < Dims3.Y; ++Y) {
    for (int X = 0; X < Dims3.X; ++X) {
      fwrite(&BlockVol.At<t>(v3i(X, Y, Z) * Strd3), sizeof(t), 1, Fp);
    }}}
    //f64 Ps = PSNR(BackupVol, BlockVol);
  } mg_EndFor3
  //fclose(Fp2);
  WriteBuffer("refinement.raw", VolRefinement.Buffer);
  WriteBuffer("dataout.raw", DataOut.Buffer);
  printf("Coeff count = %lld\n", CoeffCount);
}

mg_T(t) void
TestBlockGeneration2D(
  cstr RawFile, const v3i& N3, dtype DType, const v3i& BlockSize3, int NLevels,
  f64 NormThreshold)
{
  if constexpr(is_same_type<t, f64>::Value)
    mg_Assert(DType == dtype::float64);
  if constexpr(is_same_type<t, f32>::Value)
    mg_Assert(DType == dtype::float32);
  // TODO: use memmap to read file
  // TODO: Write a adaptor for volume using memmap
  wav_basis_norms Wn = GetCdf53Norms(NLevels);
  mg_RAII(volume, Vol, ReadVolume(RawFile, N3, DType, &Vol));
  mg_RAII(volume, VolRefinement, Resize(&VolRefinement, Dims(Vol), dtype::int8));
  mg_RAII(volume, DataOut, Resize(&DataOut, Dims(Vol), Vol.Type));
  v3i NBlocks3 = (N3 + BlockSize3 - 1) / BlockSize3;
  v3i BlockDims3 = BlockSize3 + v3i(1, 1, 0);
  // v3i BlockDims3 = BlockSize3;
  mg_RAII(array<extent>, Subbands, BuildSubbands(BlockDims3, NLevels, &Subbands));
  mg_RAII(array<grid>, SubbandsG, BuildSubbands(BlockDims3, NLevels, &SubbandsG));
  mg_RAII(volume, BlockVol, Resize(&BlockVol, BlockDims3, Vol.Type));
  mg_RAII(volume, BackupVol, Resize(&BackupVol, Dims(BlockVol), BlockVol.Type));
  mg_RAII(volume, WavBackupVol, Resize(&WavBackupVol, Dims(BlockVol), BlockVol.Type));
  i64 CoeffCount = 0;
  // Write the block headers
  mg_RAII(FILE*, Fp, Fp = fopen("blocks.raw", "wb"), if (Fp) fclose(Fp));
  fwrite(&N3, sizeof(N3), 1, Fp); // write the global dimensions
  fwrite(&Vol.Type, sizeof(Vol.Type), 1, Fp); // write data type
  fwrite(&BlockSize3, sizeof(BlockSize3), 1, Fp); // write block size
  mg_RAII(buffer, CompBuf, AllocBuf(&CompBuf, sizeof(t) * Prod<i64>(N3)), DeallocBuf(&CompBuf));
  bitstream Bs; InitWrite(&Bs, CompBuf);
  v3i B3;
  mg_BeginFor3(B3, v3i::Zero, NBlocks3, v3i::One) { // loop through the blocks
    Fill(Begin<t>(BlockVol), End<t>(BlockVol), t(0));
    extent BlockExt(B3 * BlockSize3, BlockSize3);
    extent BlockExtCrop = Crop(BlockExt, extent(N3));
    extent BlockExtLocal = Relative(BlockExtCrop, BlockExt);
    Copy(BlockExtCrop, Vol, BlockExtLocal, &BlockVol);
    Clone(BlockVol, &BackupVol);
    /* forward wavelet transform */
    v3i D3 = Dims(BlockExtLocal); // Dims
    v3i R3 = D3 + IsEven(D3);
    v3i S3(1); // Strides
    stack_array<v3<v3i>, 10> Dims3Stack; mg_Assert(NLevels < Size(Dims3Stack));
    for (int I = 0; I < NLevels; ++I) {
      FLiftCdf53OldX((t*)BlockVol.Buffer.Data, BlockSize3, v3i(I));
      FLiftCdf53OldY((t*)BlockVol.Buffer.Data, BlockSize3, v3i(I));
      //FLiftCdf53X<f64>(grid(v3i(0), v3i(D3.X, D3.Y, 1), S3), BlockSize3, lift_option::Normal, &BlockVol);
      //FLiftCdf53Y<f64>(grid(v3i(0), v3i(R3.X, D3.Y, 1), S3), BlockSize3, lift_option::Normal, &BlockVol);
      Dims3Stack[I] = v3(v3i(D3.X, D3.Y, 1), v3i(R3.X, D3.Y, 1), v3i(R3.X, R3.Y, 1));
      D3 = (R3 + 1) / 2;
      R3 = D3 + IsEven(D3);
      S3 = S3 * 2;
    }
    Clone(BlockVol, &WavBackupVol);
    /* compress */
    using itype = typename traits<t>::integral_t;
    using utype = typename traits<itype>::unsigned_t;
    v3i ZfpBlockDims3(4, 4, 1);
    v3i NZfpBlocks3 = (BlockDims3 + ZfpBlockDims3 - 1) / ZfpBlockDims3;
    v3i RealBlockDims3 = Min(N3 - B3 * BlockSize3, BlockDims3);
    // TODO: there is a bug if the tile is less than 32^3
    v3i Z3;
    mg_BeginFor3(Z3, v3i::Zero, NZfpBlocks3, v3i::One) { // through zfp blocks
      t     ZfpBlockFloats[4 * 4] = {}; buffer_t BufFloat(ZfpBlockFloats);
      itype ZfpBlockInts  [4 * 4] = {}; buffer_t BufInts (ZfpBlockInts  );
      utype ZfpBlockUInts [4 * 4] = {}; buffer_t BufUInts(ZfpBlockUInts );
      v3i D3 = Z3 * ZfpBlockDims3;
      v3i RealZfpBlockDims3 = Min(RealBlockDims3 - D3, ZfpBlockDims3);
      v3i S3;
      mg_BeginFor3(S3, v3i::Zero, RealZfpBlockDims3, v3i::One) { // through samples
        ZfpBlockFloats[Row(BlockDims3, S3)] = BlockVol.At<t>(D3 + S3);
      } mg_EndFor3 // end sample loop
      PadBlock2D(ZfpBlockFloats, RealZfpBlockDims3.XY);
      // TODO: quantize
      int Prec = sizeof(t) * 8 - 1 - 2 /* 2 == NDims */;
      int EMax = Quantize(Prec, BufFloat, &BufInts);
      Write(&Bs, EMax + traits<t>::ExpBias, traits<t>::ExpBits);
      //fwrite((i32*)TileVolQ.Buffer.Data + S, Prod(BlockDims3) * sizeof(i32), 1, Fp);
      ForwardZfp2D<itype, 4>(ZfpBlockInts);
      ForwardShuffle2D<itype, utype, 4>(ZfpBlockInts, ZfpBlockUInts);
      int NBitplanes = Prec + 1 + 2 /* 2 == NDims */;
      int Bp = NBitplanes - 1;
      i8 N = 0;
      while (Bp >= 0) { // through bit planes
        Encode<utype, 2, 4>(ZfpBlockUInts, Bp, traits<i64>::Max, N, &Bs);
        --Bp;
      } // end bit plane loop
    } mg_EndFor3 // end zfp block loop, DONE compression
    /* go through each subband, compute the norm of wavelet coefficients x basis norm */
    Fill(Begin<t>(BlockVol), End<t>(BlockVol), t(0));
    int LastSb = 0;
    for (int I = 0; I < Size(SubbandsG); ++I) {
      auto Ext = SubbandsG[I];
      v3i Lvl3 = SubbandToLevel(2, I);
      f64 Score = 0;
      if (I == 0) {
        Score = Wn.ScalNorms[NLevels - 1];
        Score = Score * Score;
      } else {
        int LMax = Max(Lvl3.X, Lvl3.Y);
        f64 Sx = (Lvl3.X == LMax) ? Wn.WaveNorms[NLevels - LMax] : Wn.ScalNorms[NLevels - LMax];
        f64 Sy = (Lvl3.Y == LMax) ? Wn.WaveNorms[NLevels - LMax] : Wn.ScalNorms[NLevels - LMax];
        Score = Sx * Sy;
      }
      /* copy the relevant coefficients over */
      for (auto It  = Begin<t>(Ext, WavBackupVol), It2 = Begin<t>(Ext, BlockVol);
                It !=   End<t>(Ext, WavBackupVol); ++It, ++It2)
      {
        f64 FinalScore = Score * fabs(*It);
        if (FinalScore >= NormThreshold) {
          *It2 = *It;
          LastSb = I;
          ++CoeffCount;
        }
      }
    }
    /* compute the relevant resolution for the block (stored in Dims3) */
    v3i Strd3O = Strd(SubbandsG[LastSb]);
    v3i SbLvl3 = v3i(SubbandToLevel(2, LastSb, true).XY, 0);
    v3i Denom3 = (1 << SbLvl3);
    if (Denom3.X == 1) Denom3.Y = 1;
    v3i Strd3 = Strd3O / Denom3;
    v3i Dims3 = (Dims(BlockVol) + Strd3 - 1) / Strd3;
    // TODO: check that this code works for boundary blocks with dims < BlockSize
    printf("dims = %d %d %d\n", Dims3.X, Dims3.Y, Dims3.Z);
    Fill(Begin<i8>(BlockExtCrop, VolRefinement), End<i8>(BlockExtCrop, VolRefinement), i8(LastSb));
    //printf("Last sb = %d\n", LastSb);
    // TODO: inverse to LastSb only
    /* inverse transform to reconstruct the block */
    for (int I = NLevels - 1; I >= 0; --I) {
      S3 = S3 / 2;
      ILiftCdf53OldY((t*)BlockVol.Buffer.Data, BlockSize3, v3i(I));
      ILiftCdf53OldX((t*)BlockVol.Buffer.Data, BlockSize3, v3i(I));
      // ILiftCdf53Y<f64>(grid(v3i(0), Dims3Stack[I].Y, S3), BlockSize3, lift_option::Normal, &BlockVol);
      // ILiftCdf53X<f64>(grid(v3i(0), Dims3Stack[I].X, S3), BlockSize3, lift_option::Normal, &BlockVol);
    }
    Copy(Relative(BlockExtCrop, BlockExt), BlockVol, BlockExtCrop, &DataOut);
    // file format:
    // dx, dy, dz, data for block 1
    // dx, dy, dz, data for block 2
    // ...
    // dx, dy, dz, data for block n
    // TODO: don't need 32 bit for the dims
    /* dump the raw values at the computed resolution (Dims3) */
    fwrite(&Dims3, sizeof(Dims3), 1, Fp);
    for (int Y = 0; Y < Dims3.Y; ++Y) {
    for (int X = 0; X < Dims3.X; ++X) {
      fwrite(&BlockVol.At<t>(v3i(X, Y, 0) * Strd3), sizeof(t), 1, Fp);
    }}
    //f64 Ps = PSNR(BackupVol, BlockVol);
  } mg_EndFor3 // end block loop
  //fclose(Fp2);
  WriteBuffer("refinement.raw", VolRefinement.Buffer);
  WriteBuffer("dataout.raw", DataOut.Buffer);
  printf("Coeff count = %lld\n", CoeffCount);
}

int main() {
  cstr RawFile = "D:/Datasets/2D/Slices/MIRANDA-PRESSURE-[384-384]-Float64.raw";
  v3i N3(384, 384, 1);
  int NLevels = 4;
  v3i BlockSize3(32, 32, 1);
  f64 NormThreshold = 0.2;
  TestBlockGeneration2D<f64>(
    RawFile, N3, dtype::float64, BlockSize3, NLevels, NormThreshold);
  return 0;
}