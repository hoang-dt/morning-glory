#include "mg_bitstream.h"
#include "mg_expected.h"
#include "mg_file_format.h"
#include "mg_scopeguard.h"
#include "mg_math.h"
#include "mg_signal_processing.h"
#include "mg_wavelet.h"
#include "mg_zfp.h"
#include <stdio.h>

// TODO: add the variable-size chunk mode
// TODO: zip each chunk
// TODO: measure the wasted space in each chunk
// TODO: make sure that incomplete chunks can be decoded correctly
// TODO: add an option to "pack" the chunks after encoding

namespace mg {

#if defined(mg_CollectStats)
void Log(cstr FileName) {
  FILE* Fp = fopen(FileName, "w");
  for (int Sb = 0; Sb < Size(FStats.SbStats); ++Sb) {
    fprintf(Fp, "Subband %d ----------------\n", Sb);
    const subband_stats& SbStats = FStats.SbStats[Sb];
    for (int Tl = 0; Tl < Size(SbStats.TlStats); ++Tl) {
      fprintf(Fp, "    Tile %d++++++++++++++++\n", Tl);
      const tile_stats& TlStats = SbStats.TlStats[Tl];
      for (int I = 0; I < Size(TlStats.EMaxes); ++I) {
        fprintf(Fp, "%d ", TlStats.EMaxes[I]);
      }
      fprintf(Fp, "\n");
      for (int Ck = 0; Ck < Size(TlStats.CkStats); ++Ck) {
        fprintf(Fp, "        Chunk %d: %d %d\n",
                Ck, TlStats.CkStats[Ck].Where, TlStats.CkStats[Ck].ActualSize);
      }
    }
  }
  fclose(Fp);
}
#endif

struct tile_data {
  bitstream Bs;
  v3i Tile;
  v3i RealDims;
  v3i NBlocksInTile;
  int Subband;
  i64 LocalId;
  i64 GlobalId;
  typed_buffer<f64> Floats;
  typed_buffer<i64> Ints;
  typed_buffer<u64> UInts;
  typed_buffer<i16> EMaxes;
  typed_buffer<i8> Ns;
  typed_buffer<i8> Ms;
  typed_buffer<bool> InnerLoops;
};

i64 NTilesInSubbands(const file_format& Ff, int FromSb, int ToSb) {
  i64 NTiles = 0;
  for (int S = FromSb; S < ToSb; ++S) {
    v3i SbDims = Extract3Ints64(Ff.Subbands[S].DimsCompact);
    NTiles += Prod<i64>((SbDims + Ff.TileDims - 1) / Ff.TileDims);
  }
  return NTiles;
}

// TODO: refactor to separate the copying of tile data from the encoding
template <typename t>
void CopyBlockForward(const file_format& Ff, tile_data* Tl, v3i Block, int K) {
  v3i Dims = Extract3Ints64(Ff.Volume.DimsCompact);
  v3i RealBlockDims = Min(Ff.TileDims - Block, ZDims);
  const t* Data = (t*)Ff.Volume.Buffer.Data;
  v3i Voxel;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, Tl->Tile + Block + Voxel);
    i64 J = K + XyzToI(ZDims, Voxel);
    Tl->Floats[J] = Data[I];
  } mg_EndFor3
  PadBlock(Tl->Floats.Data, RealBlockDims.X, 1);
  PadBlock(Tl->Floats.Data, RealBlockDims.Y, 4);
  PadBlock(Tl->Floats.Data, RealBlockDims.Z, 16);
}

// TODO: refactor to separate the copying from the decoding
template <typename t>
void CopyBlockInverse(file_format* Ff, tile_data* Tl, v3i Block, int K) {
  v3i Dims = Extract3Ints64(Ff->Volume.DimsCompact);
  v3i RealBlockDims = Min(Tl->RealDims - Block, ZDims);
  v3i Voxel;
  t* Data = (t*)Ff->Volume.Buffer.Data;
  mg_BeginFor3(Voxel, v3i::Zero(), RealBlockDims, v3i::One()) {
    i64 I = XyzToI(Dims, Tl->Tile + Block + Voxel);
    i64 J = K + XyzToI(ZDims, Voxel);
    Data[I] = Tl->Floats[J];
  } mg_EndFor3
}

void WriteEMax(int EMax, int ToleranceExp, bitstream* Bs) {
  if (0 <= EMax - ToleranceExp + 1) {
    Write(Bs, 1);
    // TODO: for now we don't care if the exponent is 2047 which represents Inf
    // or NaN
    Write(Bs, EMax + Traits<f64>::ExpBias, Traits<f64>::ExpBits);
  } else {
    Write(Bs, 0);
  }
}

bool ReadEMax(bitstream* Bs, int ToleranceExp, i16* EMax) {
  if (Read(Bs)) {// significant
    *EMax = (i16)Read(Bs, Traits<f64>::ExpBits) - Traits<f64>::ExpBias;
    return true;
  }
  *EMax = (i16)(ToleranceExp - 2);
  return false;
}

// TODO: error handling
// TODO: minimize fopen calls
ff_err WriteChunk(const file_format& Ff, tile_data* Tl, int Ci) {
  mg_Assert(Size(Tl->Bs) > 0);
  // TODO: cache the file names if the formatting turns out to be slow
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", Ff.FileName, Ci);
  FILE* Fp = fopen(FileNameBuf, "r+b");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  if (!Fp) { // if the file is not present, create it
    Fp = fopen(FileNameBuf, "wb");
    fwrite(Ff.Meta, Ff.MetaBytes, 1, Fp);
    fwrite(Ff.TileHeaders.Data, Bytes(Ff.TileHeaders), 1, Fp);
  } else { // file exists, go to the end
    mg_FSeek(Fp, 0, SEEK_END); // TODO: this prevents parallelization in file I/O
  }
  u64 Where = mg_FTell(Fp);
#if defined(mg_CollectStats)
  tile_stats& Ts = FStats.SbStats[Tl->Subband].TlStats[Tl->LocalId];
  PushBack(&(Ts.CkStats), chunk_stats{(int)Where, (int)Size(Tl->Bs)});
#endif
  fwrite(Tl->Bs.Stream.Data, Ff.ChunkBytes, 1, Fp);
  mg_FSeek(Fp, Ff.MetaBytes + sizeof(u64) * Tl->GlobalId, SEEK_SET);
  fwrite(&Where, sizeof(Where), 1, Fp);
  Flush(&Tl->Bs);
  InitWrite(&Tl->Bs, Tl->Bs.Stream);
  return mg_Error(ff_err_code::NoError);
}

// TODO: error handling
// TODO: minimize file opening
// TODO: the tile size should depend on the precision at some level, to reduce
// internal fragmentation
template <typename t>
ff_err WriteTile(const file_format& Ff, tile_data* Tl) {
#if defined(mg_CollectStats)
  tile_stats& Ts = FStats.SbStats[Tl->Subband].TlStats[Tl->LocalId];
  Ts.LocalId = Tl->LocalId;
#endif
  int Ci = 0; // chunk id
  InitWrite(&Tl->Bs, Tl->Bs.Stream);
  for (int Bp = Ff.Prec; Bp >= 0; --Bp) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), Tl->RealDims, ZDims) {
      int Bi = XyzToI(Tl->NBlocksInTile, Block / ZDims);
      int K = XyzToI(Tl->NBlocksInTile, Block / ZDims) * Prod(ZDims);
      /* Copy the block data into the tile's buffer */
      if (Bp == Ff.Prec) {
        CopyBlockForward<t>(Ff, Tl, Block, K);
        Tl->EMaxes[Bi] =
          (i16)Quantize((byte*)&Tl->Floats[K], Prod(ZDims),
                        Ff.Prec - 2, (byte*)&Tl->Ints[K], Ff.Volume.Type);
#if defined(mg_CollectStats)
        PushBack(&(Ts.EMaxes), Tl->EMaxes[Bi]);
#endif
        WriteEMax(Tl->EMaxes[Bi], Exponent(Ff.Tolerance), &Tl->Bs);
        ForwardBlockTransform(&Tl->Ints[K]);
        ForwardShuffle(&Tl->Ints[K], &Tl->UInts[K]);
      }
      /* Encode and write chunks */
      bool DoEncode = Ff.Prec - Bp <= Tl->EMaxes[Bi] - Exponent(Ff.Tolerance) + 1;
      bool LastChunk = (Bp == 0) && (Bi + 1 == Prod(Tl->NBlocksInTile));
      bool FullyEncoded = true;
      do {
        if (DoEncode) {
          FullyEncoded =
            EncodeBlock(&Tl->UInts[K], Bp, Ff.ChunkBytes * 8, Tl->Ns[Bi],
                        Tl->Ms[Bi], Tl->InnerLoops[Bi], &Tl->Bs);
        }
        bool ChunkComplete = Size(Tl->Bs) >= Ff.ChunkBytes;
        if (Size(Tl->Bs) > 0 && (ChunkComplete || LastChunk)) {
          WriteChunk(Ff, Tl, Ci++);
        }
      } while (!FullyEncoded);
    } mg_EndFor3
  }
  return mg_Error(ff_err_code::NoError);
}

// TODO: use the freelist allocator
// TODO: use aligned memory allocation
// TODO: try reusing the memory buffer
// TODO: write the tile in Z order
template <typename t>
ff_err WriteSubband(const file_format& Ff, int Sb) {
  v3i SbPos3 = Extract3Ints64(Ff.Subbands[Sb].PosCompact);
  v3i SbDims = Extract3Ints64(Ff.Subbands[Sb].DimsCompact);
  v3i Tile;
#if defined(mg_CollectStats)
  FStats.SbStats[Sb].NumTiles3 = (SbDims + Ff.TileDims - 1) / Ff.TileDims;
  Resize(&FStats.SbStats[Sb].TlStats, Prod(FStats.SbStats[Sb].NumTiles3));
#endif
  mg_BeginFor3(Tile, SbPos3, SbPos3 + SbDims, Ff.TileDims) {
    v3i NTilesInSb = (SbDims + Ff.TileDims - 1) / Ff.TileDims;
    tile_data Tl;
    Tl.Tile = Tile;
    Tl.RealDims = Min(SbPos3 + SbDims - Tile, Ff.TileDims);
    Tl.LocalId = XyzToI(NTilesInSb, (Tile - SbPos3) / Ff.TileDims);
    Tl.GlobalId = NTilesInSubbands(Ff, 0, Sb) + Tl.LocalId;
    Tl.NBlocksInTile = ((Tl.RealDims + ZDims) - 1) / ZDims;
    Tl.Subband = Sb;
    AllocBufT(&Tl.Floats, Prod(Ff.TileDims));
    AllocBufT(&Tl.Ints, Prod(Ff.TileDims));
    AllocBufT(&Tl.UInts, Prod(Ff.TileDims));
    AllocBufT(&Tl.EMaxes, Prod(Tl.NBlocksInTile));
    AllocBufT0(&Tl.Ns, Prod(Tl.NBlocksInTile));
    AllocBufT0(&Tl.Ms, Prod(Tl.NBlocksInTile));
    AllocBufT0(&Tl.InnerLoops, Prod(Tl.NBlocksInTile));
    AllocBuf(&Tl.Bs.Stream, Ff.ChunkBytes + BufferSize(Tl.Bs));
    mg_CleanUp(0, DeallocBufT(&Tl.Floats);
                  DeallocBufT(&Tl.Ints);
                  DeallocBufT(&Tl.UInts);
                  DeallocBufT(&Tl.EMaxes);
                  DeallocBufT(&Tl.Ns);
                  DeallocBufT(&Tl.Ms);
                  DeallocBufT(&Tl.InnerLoops);
                  DeallocBuf(&Tl.Bs.Stream));
    ff_err Err = WriteTile<t>(Ff, &Tl);
    if (ErrorOccurred(Err))
      return Err;
  } mg_EndFor3
  return mg_Error(ff_err_code::NoError);
}

// TODO: separate the reading of meta data from the decoding so that the client
// can manage their own memory

// TODO: change AllocateTypedBuffer to AllocTypedBuf
ff_err Finalize(file_format* Ff, file_format::mode Mode) {
  /* Only support float32 and float64 for now */
  if (Ff->Volume.Type != data_type::float32 &&
      Ff->Volume.Type != data_type::float64)
    return mg_Error(ff_err_code::TypeNotSupported);
  v3i Dims = Extract3Ints64(Ff->Volume.DimsCompact);
  BuildSubbands(Dims, Ff->NLevels, &Ff->Subbands);
  v3i Sb0Dims = Extract3Ints64(Ff->Subbands[0].DimsCompact);
  if (!(Ff->TileDims >= ZDims || Sb0Dims >= Ff->TileDims))
    return mg_Error(ff_err_code::InvalidTileDims);
  /* Chunk size must be large enough to store all the EMaxes in a tile */
  if (Ff->Volume.Type == data_type::float32) {
    if (Ff->ChunkBytes * 8 < Prod(Ff->TileDims / ZDims) * Traits<f32>::ExpBits)
      return mg_Error(ff_err_code::InvalidChunkSize);
  } else if (Ff->Volume.Type == data_type::float64) {
    if (Ff->ChunkBytes * 8 < Prod(Ff->TileDims / ZDims) * Traits<f64>::ExpBits)
      return mg_Error(ff_err_code::InvalidChunkSize);
  }
  i64 NTiles = NTilesInSubbands(*Ff, 0, Size(Ff->Subbands));
  AllocBufT0(&Ff->TileHeaders, NTiles + 1);
  if (Mode == file_format::mode::Read) {
    /* allocate memory for the linked list */
    AllocBufT(&Ff->Chunks, Size(Ff->Subbands));
    for (int Sb = 0; Sb < Size(Ff->Subbands); ++Sb) {
      v3i SbDims3 = Extract3Ints64(Ff->Subbands[Sb].DimsCompact);
      v3i NTiles3 = (SbDims3 + Ff->TileDims - 1) / Ff->TileDims;
      AllocBufT(&Ff->Chunks[Sb], Prod<i64>(NTiles3));
      for (i64 Ti = 0; Ti < Size(Ff->Chunks[Sb]); ++Ti)
        new (&Ff->Chunks[Sb][Ti]) linked_list<buffer>;
    }
    AllocBuf(&Ff->Volume.Buffer, Prod<i64>(Dims) * SizeOf(Ff->Volume.Type));
  }
  return mg_Error(ff_err_code::NoError);
}

// TODO: think of a name for the file format
// TODO: set the compressed_file by default to the name of the input file
// TODO: add an (optional) post-processing step to merge all compressed files
// TODO: study the sqlite format
// TODO: write a special lifting procedure that takes into account the box
//       containing changed coefficients only
// TODO: provide more context info for returned errors

/* Write meta data (and the tile headers) to the first file */
int FormatMeta(file_format* Ff, const metadata& Meta) {
  int N = snprintf(Ff->Meta, sizeof(Ff->Meta), "WZ %d.%d\n",
                   Ff->Major, Ff->Minor); // version
  N += snprintf(Ff->Meta + N, sizeof(Ff->Meta) - N, "000 bytes\n"); // header size
  auto Lambda = [&Ff, &MetaBuf = Ff->Meta, &Meta](int N) {
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "name = %s\n", Meta.Name);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "field = %s\n", Meta.Field);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "dimensions = %d %d %d\n",
                  Meta.Dims.X, Meta.Dims.Y, Meta.Dims.Z);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "type = %s\n",
                  ToString(Meta.Type).Ptr);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "num levels = %d\n",
                  Ff->NLevels);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "tile dims = %d %d %d\n",
                  Ff->TileDims.X, Ff->TileDims.Y, Ff->TileDims.Z);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "chunk bytes = %d\n",
                  Ff->ChunkBytes);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "precision = %d\n", Ff->Prec);
    N += snprintf(MetaBuf + N, sizeof(MetaBuf) - N, "tolerance = %f\n",
                  Ff->Tolerance);
    return N;
  };
  int M = Lambda(N) + 1; // + 1 for the NULL char at the end
  N = snprintf(Ff->Meta, sizeof(Ff->Meta), "WZ %d.%d\n", Ff->Major, Ff->Minor);
  N += snprintf(Ff->Meta + N, sizeof(Ff->Meta) - N, "%03d bytes\n", M);
  N = Lambda(N) + 1;
  mg_Assert(M == N);
  return N;
}

ff_err ParseMeta(file_format* Ff, metadata* Meta) {
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", Ff->FileName, 0);
  FILE* Fp = fopen(FileNameBuf, "rb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  if (!Fp)
    return mg_Error(ff_err_code::FileOpenFailed, "%s", FileNameBuf);
  if (fscanf(Fp, "WZ %d.%d\n", &Ff->Major, &Ff->Minor) != 2)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Version corrupted");
  // TODO: check for version incompatibility
  if (fscanf(Fp, "%03d bytes\n", &Ff->MetaBytes) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Meta Bytes corrupted");
  if (fscanf(Fp, "name = %s\n", Meta->Name) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Name corrupted");
  if (fscanf(Fp, "field = %s\n", Meta->Field) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Field corrupted");
  if (fscanf(Fp, "dimensions = %d %d %d\n",
             &Meta->Dims.X, &Meta->Dims.Y, &Meta->Dims.Z) != 3)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Dims corrupted");
  Ff->Volume.DimsCompact = Stuff3Ints64(Meta->Dims);
  char Type[16];
  if (fscanf(Fp, "type = %s\n", Type) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Type corrupted");
  Ff->Volume.Type = Meta->Type = StringTo<data_type>()(str_ref(Type));
  if (fscanf(Fp, "num levels = %d\n", &Ff->NLevels) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Num Levels corrupted");
  if (fscanf(Fp, "tile dims = %d %d %d\n",
      &Ff->TileDims.X, &Ff->TileDims.Y, &Ff->TileDims.Z) != 3)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Tile Dims corrupted");
  if (fscanf(Fp, "chunk bytes = %d\n", &Ff->ChunkBytes) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Chunk Bytes corrupted");
  if (fscanf(Fp, "precision = %d\n", &Ff->Prec) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Precision corrupted");
  if (fscanf(Fp, "tolerance = %lf\n", &Ff->Tolerance) != 1)
    return mg_Error(ff_err_code::ParseFailed, "Meta data: Tolerance corrupted");
  return mg_Error(ff_err_code::NoError);
}

// TODO: write to an existing file
ff_err Encode(file_format* Ff, const metadata& Meta) {
  ff_err Err = Finalize(Ff, file_format::mode::Write);
  if (ErrorOccurred(Err))
    return Err;
  if (Ff->DoExtrapolation) {
    // TODO
  }
  if (Ff->NLevels > 0)
    Cdf53Forward(&(Ff->Volume), Ff->NLevels);
#if defined(mg_CollectStats)
  Resize(&FStats.SbStats, Size(Ff->Subbands));
#endif
  Ff->MetaBytes = FormatMeta(Ff, Meta);
  mg_Assert(Ff->MetaBytes == 1 + (int)strnlen(Ff->Meta, sizeof(Ff->Meta)));
  for (int Sb = 0; Sb < Size(Ff->Subbands); ++Sb) {
    if (Ff->Volume.Type == data_type::float64) {
      if (ErrorOccurred(Err = WriteSubband<f64>(*Ff, Sb)))
        return Err;
    } else if (Ff->Volume.Type == data_type::float32) {
      if (ErrorOccurred(Err = WriteSubband<f32>(*Ff, Sb)))
        return Err;
    } else {
      mg_Abort("Type not supported");
    }
  }
  return mg_Error(ff_err_code::NoError);
}

void SetTileDims(file_format* Ff, v3i TileDims) {
  Ff->TileDims = TileDims;
}
void SetChunkBytes(file_format* Ff, int ChunkBytes) {
  Ff->ChunkBytes = ChunkBytes;
}
void SetFileName(file_format* Ff, cstr FileName) {
  Ff->FileName = FileName;
}
void SetTolerance(file_format* Ff, f64 Tolerance) {
  Ff->Tolerance = Tolerance;
}
void SetPrecision(file_format* Ff, int Precision) {
  Ff->Prec = Precision;
}
void SetVolume(file_format* Ff, byte* Data, v3i Dims, data_type Type) {
  Ff->Volume.Buffer.Data = Data;
  Ff->Volume.Buffer.Bytes = SizeOf(Type) * Prod<i64>(Dims);
  Ff->Volume.DimsCompact = Stuff3Ints64(Dims);
  Ff->Volume.Type = Type;
}
void SetWaveletTransform(file_format* Ff, int NLevels) {
  Ff->NLevels = NLevels;
}
void SetExtrapolation(file_format* Ff, bool DoExtrapolation) {
  Ff->DoExtrapolation = DoExtrapolation;
}

/* Read the next chunk from disk */
ff_err ReadNextChunk(file_format* Ff, tile_data* Tl, buffer* ChunkBuf) {
  mg_Assert(ChunkBuf->Bytes == Ff->ChunkBytes);
  auto& ChunkList = Ff->Chunks[Tl->Subband][Tl->LocalId];
  int ChunkId = int(Size(ChunkList));
  char FileNameBuf[256];
  snprintf(FileNameBuf, sizeof(FileNameBuf), "%s%d", Ff->FileName, ChunkId);
  FILE* Fp = fopen(FileNameBuf, "rb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  /* Read the chunk header */
  u64 Where = 0;
  if (Fp) {
    mg_FSeek(Fp, Ff->MetaBytes + sizeof(u64) * Tl->GlobalId, SEEK_SET);
    if (fread(&Where, sizeof(u64), 1, Fp) != 1)
      return mg_Error(ff_err_code::FileReadFailed);
  } else {
    return mg_Error(ff_err_code::FileOpenFailed);
  }
  /* Read the chunk data */
  if (Where > 0) {
#if defined(mg_CollectStats)
    tile_stats& Ts = FStats.SbStats[Tl->Subband].TlStats[Tl->LocalId];
    PushBack(&(Ts.CkStats), chunk_stats{(int)Where, 0});
#endif
    mg_FSeek(Fp, Where, SEEK_SET);
    if (fread(ChunkBuf->Data, ChunkBuf->Bytes, 1, Fp) == 1) {
      auto ChunkIt = PushBack(&ChunkList, *ChunkBuf);
      InitRead(&Tl->Bs, *ChunkIt);
    } else { // cannot read the chunk in the file
      return mg_Error(ff_err_code::FileReadFailed);
    }
  } else { // the chunk does not exist
    return mg_Error(ff_err_code::ChunkReadFailed);
  }
  return error(ff_err_code::NoError);
}

// TODO: add signature to the .h file
template <typename t>
ff_err ReadSubband(file_format* Ff, int Sb) {
  v3i SbPos3 = Extract3Ints64(Ff->Subbands[Sb].PosCompact);
  v3i SbDims = Extract3Ints64(Ff->Subbands[Sb].DimsCompact);
  v3i Tile;
#if defined(mg_CollectStats)
  FStats.SbStats[Sb].NumTiles3 = (SbDims + Ff->TileDims - 1) / Ff->TileDims;
  Resize(&FStats.SbStats[Sb].TlStats, Prod(FStats.SbStats[Sb].NumTiles3));
#endif
  mg_BeginFor3(Tile, SbPos3, SbPos3 + SbDims, Ff->TileDims) {
    v3i NTilesInSb3 = (SbDims + Ff->TileDims - 1) / Ff->TileDims;
    tile_data Tl;
    Tl.Tile = Tile;
    Tl.RealDims = Min(SbPos3 + SbDims - Tile, Ff->TileDims);
    Tl.LocalId = XyzToI(NTilesInSb3, (Tile - SbPos3) / Ff->TileDims);
    Tl.GlobalId = NTilesInSubbands(*Ff, 0, Sb) + Tl.LocalId;
    Tl.NBlocksInTile = ((Tl.RealDims + ZDims) - 1) / ZDims;
    Tl.Subband = Sb;
    AllocBufT(&Tl.Floats, Prod(Ff->TileDims));
    AllocBufT(&Tl.Ints, Prod(Ff->TileDims));
    AllocBufT(&Tl.UInts, Prod(Ff->TileDims));
    AllocBufT(&Tl.EMaxes, Prod(Tl.NBlocksInTile));
    AllocBufT0(&Tl.Ns, Prod(Tl.NBlocksInTile));
    AllocBufT0(&Tl.Ms, Prod(Tl.NBlocksInTile));
    AllocBufT0(&Tl.InnerLoops, Prod(Tl.NBlocksInTile));
    AllocBuf(&Tl.Bs.Stream, Ff->ChunkBytes + BufferSize(Tl.Bs));
    mg_CleanUp(0, DeallocBufT(&Tl.Floats);
                  DeallocBufT(&Tl.Ints);
                  DeallocBufT(&Tl.UInts);
                  DeallocBufT(&Tl.EMaxes);
                  DeallocBufT(&Tl.Ns);
                  DeallocBufT(&Tl.Ms);
                  DeallocBufT(&Tl.InnerLoops);
                  DeallocBuf(&Tl.Bs.Stream));
    buffer ChunkBuf;
    AllocBuf(&ChunkBuf, Ff->ChunkBytes);
    ff_err Err(ff_err_code::NoError);
    while (true) {
      ff_err Err = ReadNextChunk(Ff, &Tl, &ChunkBuf);
      if (ErrorOccurred(Err) && Err.Code != ff_err_code::ChunkReadFailed &&
                                Err.Code != ff_err_code::FileOpenFailed)
        return Err;
      if (Err.Code == ff_err_code::ChunkReadFailed ||
          Err.Code == ff_err_code::FileOpenFailed)
        break;
    }
    DecompressTile<t>(Ff, &Tl);
  } mg_EndFor3
  return mg_Error(ff_err_code::NoError);
}

ff_err Decode(file_format* Ff, metadata* Meta) {
  ParseMeta(Ff, Meta);
  ff_err Err = Finalize(Ff, file_format::mode::Read);
  if (ErrorOccurred(Err))
    return Err;
#if defined(mg_CollectStats)
  Resize(&FStats.SbStats, Size(Ff->Subbands));
#endif
  for (int Sb = 0; Sb < Size(Ff->Subbands); ++Sb) {
    if (Ff->Volume.Type == data_type::float64) {
      if (ErrorOccurred(Err = ReadSubband<f64>(Ff, Sb)))
        return Err;
    } else {
      if (ErrorOccurred(Err = ReadSubband<f32>(Ff, Sb)))
        return Err;
    }
  }
  return mg_Error(ff_err_code::NoError);
}

// TODO: add an API function called ImproveTile that uses ReadNextChunk and
// DecompressTile

// TODO: add an "incremental" mode where the returned values are deltas
template <typename t>
void DecompressTile(file_format* Ff, tile_data* Tl) {
#if defined(mg_CollectStats)
  tile_stats& Ts = FStats.SbStats[Tl->Subband].TlStats[Tl->LocalId];
  Ts.LocalId = Tl->LocalId;
#endif
  InitRead(&Tl->Bs, Tl->Bs.Stream);
  const auto & ChunkList = Ff->Chunks[Tl->Subband][Tl->LocalId];
  auto ChunkIt = ConstBegin(ChunkList);
  for (int Bp = Ff->Prec; Bp >= 0; --Bp) {
    v3i Block;
    mg_BeginFor3(Block, v3i::Zero(), Tl->RealDims, ZDims) {
      int Bi = XyzToI(Tl->NBlocksInTile, Block / ZDims);
      bool DoDecode = false;
      if (Bp == Ff->Prec) {
        DoDecode = ReadEMax(&Tl->Bs, Exponent(Ff->Tolerance), &Tl->EMaxes[Bi]);
#if defined(mg_CollectStats)
        PushBack(&(Ts.EMaxes), Tl->EMaxes[Bi]);
#endif
      }
      int K = XyzToI(Tl->NBlocksInTile, Block / ZDims) * Prod(ZDims);
      DoDecode &= Ff->Prec - Bp <= Tl->EMaxes[Bi] - Exponent(Ff->Tolerance) + 1;
      bool FullyDecoded = false;
      bool LastChunk = ChunkIt == ConstEnd(ChunkList);
      bool ExhaustedBits = BitSize(Tl->Bs) >= Ff->ChunkBytes * 8;
      while (DoDecode && !FullyDecoded && !LastChunk) {
        FullyDecoded =
          DecodeBlock(&Tl->UInts[K], Bp, Ff->ChunkBytes * 8, Tl->Ns[Bi],
                      Tl->Ms[Bi], Tl->InnerLoops[Bi], &Tl->Bs);
        ExhaustedBits = BitSize(Tl->Bs) >= Ff->ChunkBytes * 8;
#if defined(mg_CollectStats)
        //if (FullyDecoded || ExhaustedBits)
        //  PushBack(&(Ts.CkStats), chunk_stats{0, (int)Size(Tl->Bs)});
#endif
        if (ExhaustedBits) {
          ++ChunkIt;
          LastChunk = ChunkIt == ConstEnd(ChunkList);
          if (!LastChunk)
            InitRead(&Tl->Bs, *ChunkIt);
        } else {
          mg_Assert(FullyDecoded);
        }
      }

      if (Bp == 0) {
        InverseShuffle(Tl->UInts.Data, Tl->Ints.Data);
        InverseBlockTransform(Tl->Ints.Data);
        Dequantize((byte*)Tl->Ints.Data, Prod(ZDims), Tl->EMaxes[Bi],
                   Ff->Prec - 1, (byte*)Tl->Floats.Data, Ff->Volume.Type);
        CopyBlockInverse<t>(Ff, Tl, Block, K);
      }
      if (LastChunk)
        goto END;
    } mg_EndFor3
  }
END:
  return;
}

void CleanUp(file_format* Ff) {
  if (Ff->Mode == file_format::mode::Read) {
    for (int S = 0; S < Size(Ff->Subbands); ++S) {
      for (int T = 0; T < Size(Ff->Chunks[S]); ++T)
        Deallocate(&Ff->Chunks[S][T]);
      DeallocBufT(&Ff->Chunks[S]);
    }
    if (Size(Ff->Chunks) > 0)
      DeallocBufT(&Ff->Chunks);
  }
  if (Size(Ff->TileHeaders) > 0)
    DeallocBufT(&Ff->TileHeaders);
}

} // namespace mg

