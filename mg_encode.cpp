#include "math.h"
#include "mg_algorithm.h"
#include "mg_array.h"
#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_bitstream.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_scopeguard.h"
#include "mg_wavelet.h"
#include "mg_types.h"

namespace mg {

// TODO: test these functions

/* Given an octree level, return the first index. The 0th level consists of 1 value at index 0. */
mg_ForceInline
int BeginIndex(int Level) {
  if (Level == 0)
    return 0;
  return (Pow8[Level] - 1) / (8 - 1);
}

mg_ForceInline
int EndIndex(int Level) {
  return BeginIndex(Level + 1);
}

/* Return the number of levels in an octree whose leaf level consists of NumValues values */
mg_ForceInline
int NumLevels(int NumValues) {
  mg_Assert(IsPow2(NumValues)); // TODO: should be IsPow8
  return Log8Floor(NumValues) + 1;
}

/* Return the octree level of the given index */
mg_ForceInline
int Level(int Idx) {
  return Log8Floor(Idx * (8 - 1) + 1);
}

/* Return the index of the parent node */
mg_ForceInline
int ParentIndex(int Idx) {
  int L = Level(Idx);
  mg_Assert(L > 0);
  return (Idx - BeginIndex(L)) / 8 + BeginIndex(L - 1);
}

/* Also quantize and convert to negabinary, and return the EMax for the block */
int CopyBlockSamplesMorton(const f64* Data, v3i Dims, int Bits, v3i BlockDims, v3i Pos,
  f64* BlockData, u64* UIntData, i8* MsbTable) {
  mg_Assert(BlockDims == v3i(16, 16, 16));
  int BlockSize = Prod<int>(BlockDims);
  f64 Max = 0;
  for (int Z = 0; Z < BlockDims.Z; ++Z) {
  for (int Y = 0; Y < BlockDims.Y; ++Y) {
  for (int X = 0; X < BlockDims.X; ++X) {
    i64 I = XyzToI(Dims, Pos + v3i(X, Y, Z));
    i32 J = EncodeMorton3(X, Y, Z);
    BlockData[J] = Data[I];
    f64 FAbs = fabs(Data[I]);
    if (FAbs > Max)
      Max = FAbs;
  }}}
  int EMax = Exponent(Max);
  //printf("%d\n", EMax);
  double Scale = ldexp(1, Bits - 1 - EMax);
  auto Mask = Traits<u64>::NegabinaryMask;
  for (int I = 0; I < BlockSize; ++I) {
    i64 IntVal = i64(Scale * BlockData[I]);
    u64 UIntVal = (u64)((IntVal + Mask) ^ Mask);
    MsbTable[I] = BitScanReverse(UIntVal);
    UIntData[I] = UIntVal;
  }
  return EMax;
}

/* Also dequantize and unconvert from negabinary */
void CopyBlockSamplesInverseMorton(const u64* UIntData, v3i Dims, int Bits, v3i BlockDims, v3i Pos,
  int EMax, f64* Data) {
  mg_Assert(BlockDims == v3i(16, 16, 16));
  double Scale = 1.0 / ldexp(1, Bits - 1 - EMax);
  auto Mask = Traits<u64>::NegabinaryMask;
  for (int Z = 0; Z < BlockDims.Z; ++Z) {
  for (int Y = 0; Y < BlockDims.Y; ++Y) {
  for (int X = 0; X < BlockDims.X; ++X) {
    i64 I = XyzToI(Dims, Pos + v3i(X, Y, Z));
    i32 J = EncodeMorton3(X, Y, Z);
    i64 IntVal = (i64)((UIntData[J] ^ Mask) - Mask);
    Data[I] = f64(IntVal * Scale);
  }}}
}

// TODO: we can (maybe) speed this up by only traversing the octree nodes that were 0 in the previous
// bit plane
// TODO: we can also detect when all the samples in the block have turned significant, then we can
// by-pass traversing the octree and simply output the raw bits
// TODO: we want to pass in the size of arrays as well for ease of debugging (not just a pointer)
void BuildSignificanceOctree(const i8* MsbTable, v3i BlockDims, int Bitplane, u8* Octree) {
  mg_Assert(BlockDims == v3i(16, 16, 16));
  int NLevels = NumLevels(Prod<int>(BlockDims)) - 1;
  mg_Assert(NLevels == 4); // assuming 16^3 blocks
  for (int L = NLevels - 1; L > 0; --L) {
    int Begin = BeginIndex(L);
    int End = EndIndex(L);
    if (L == NLevels - 1) { // last level
      for (int I = Begin; I < End; ++I) {
        int J = I - Begin;
        Octree[I] = (((u8(MsbTable[J * 8 + 0] >= Bitplane) << 0)   |
                      (u8(MsbTable[J * 8 + 1] >= Bitplane) << 1))  |
                     ((u8(MsbTable[J * 8 + 2] >= Bitplane) << 2)   |
                      (u8(MsbTable[J * 8 + 3] >= Bitplane) << 3))) |
                    (((u8(MsbTable[J * 8 + 4] >= Bitplane) << 4)   |
                      (u8(MsbTable[J * 8 + 5] >= Bitplane) << 5))  |
                     ((u8(MsbTable[J * 8 + 6] >= Bitplane) << 6)   |
                      (u8(MsbTable[J * 8 + 7] >= Bitplane) << 7)));
      }
    }
    for (int I = Begin, ParentIdx = ParentIndex(Begin); I < End; I += 8, ++ParentIdx) {
      Octree[ParentIdx] = (((u8(Octree[I + 0] > 0) << 0)   |
                            (u8(Octree[I + 1] > 0) << 1))  |
                           ((u8(Octree[I + 2] > 0) << 2)   |
                            (u8(Octree[I + 3] > 0) << 3))) |
                          (((u8(Octree[I + 4] > 0) << 4)   |
                            (u8(Octree[I + 5] > 0) << 5))  |
                           ((u8(Octree[I + 6] > 0) << 6)   |
                            (u8(Octree[I + 7] > 0) << 7)));

    }
  }
}

// Remember the octree does not store the last level (16^3 samples)
// PrevOctree stores the octree up to the previous bit plane
// TODO: maybe maintaining a list is faster?
void EncodeBlock(const u64* Block, v3i BlockDims, int Bitplane,
  const u8* PrevOctree, const u8* Octree, bitstream* Bs, v3i Pos)
{
  mg_Assert(BlockDims == v3i(16, 16, 16));
  int NLevels = NumLevels(Prod<int>(BlockDims));
  mg_Assert(NLevels == 5);
  int BeginIndexLastLevel = BeginIndex(NLevels - 1);
  int Indices[4] = { 0 }; // save the indices through the levels
  int Children[4] = { 0 }; // save the current children through the levels
  int CurrLevel = 0, CurrIndex = 0, CurrChild = 0;
  Write(Bs, (Octree[0] != 0));
  if (Octree[0] == 0)
    return;
  while (CurrLevel >= 0) {
    bool IsChildSignificant = (Octree[CurrIndex] >> CurrChild) & 1u;
    bool IsChildSignificantPrev = (PrevOctree[CurrIndex] >> CurrChild) & 1u;
    if (IsChildSignificant) {
      if (!IsChildSignificantPrev)  // only write 1-bit if the child was not previously significant
        Write(Bs, 1);
      int ChildIndex = CurrIndex * 8 + CurrChild + 1;
      if (ChildIndex >= BeginIndexLastLevel) { // child is on last level where each node is a sample
        Write(Bs, 1 & (Block[ChildIndex - BeginIndexLastLevel] >> Bitplane));
        ++CurrChild; // move to the next child since we can't recurse
      } else { // not the last level, recurse down
        Indices [CurrLevel] = CurrIndex; // save the current index
        Children[CurrLevel] = CurrChild; // save the current children
        ++CurrLevel;
        CurrChild = 0;
        CurrIndex = ChildIndex;
      }
    } else { // child is insignificant, write 0 and go to next child
      Write(Bs, 0);
      ++CurrChild;
    }
    while (CurrChild >= 8) { // rewind, go up the levels
      --CurrLevel;
      if (CurrLevel < 0)
        break;
      CurrIndex = Indices[CurrLevel];
      CurrChild = ++Children[CurrLevel]; // go to next children
    }
  }
}

/* Update the octree as we decode */
void DecodeBlock(u64* Block, v3i BlockDims, int Bitplane, const u8* PrevOctree, u8* Octree,
  bitstream* Bs, v3i Pos) {
  mg_Assert(BlockDims == v3i(16, 16, 16));
  int NLevels = NumLevels(Prod<int>(BlockDims));
  mg_Assert(NLevels == 5);
  int BeginIndexLastLevel = BeginIndex(NLevels - 1);
  int Indices[4] = { 0 }; // save the indices through the levels
  int Children[4] = { 0 }; // save the current children through the levels
  int CurrLevel = 0, CurrIndex = 0, CurrChild = 0;
  int BlockSignificant = Read(Bs);
  if (BlockSignificant == 0) return;
  while (CurrLevel >= 0) {
    bool IsChildSignificantPrev = (PrevOctree[CurrIndex] >> CurrChild) & 1u;
    bool IsChildSignificant = IsChildSignificantPrev;
    if (!IsChildSignificantPrev)
      IsChildSignificant = Read(Bs);
    if (IsChildSignificant) {
      if (!IsChildSignificantPrev)
        Octree[CurrIndex] |= 1 << CurrChild;
      int ChildIndex = CurrIndex * 8 + CurrChild + 1;
      mg_Assert(ChildIndex - BeginIndexLastLevel < 16*16*16);
      if (ChildIndex >= BeginIndexLastLevel) { // child is on last level where each node is a sample
        Block[ChildIndex - BeginIndexLastLevel] |= Read(Bs) << Bitplane;
        ++CurrChild; // move to the next child since we can't recurse
      } else { // not the last level, recurse down
        Indices [CurrLevel] = CurrIndex; // save the current index
        Children[CurrLevel] = CurrChild; // save the current children
        ++CurrLevel;
        CurrChild = 0;
        CurrIndex = ChildIndex;
      }
    } else { // child is insignificant, write 0 and go to next child
      ++CurrChild;
    }
    while (CurrChild >= 8) { // rewind, go up the levels
      --CurrLevel;
      if (CurrLevel < 0)
        break;
      CurrIndex = Indices[CurrLevel];
      CurrChild = ++Children[CurrLevel]; // go to next children
    }
  }
}

/* Each tile is 32x32x32, but we encode each block of 16x16x16 independently within a tile */
void Encode(const f64* Data, v3i Dims, v3i TileDims, int Bits, const dynamic_array<Block>& Subbands,
  cstr FileName)
{
  FILE* Fp = fopen(FileName, "wb");
  mg_Assert(TileDims == v3i(32, 32, 32));
  v3i BlockDims(16, 16, 16);
  mg_Assert(BlockDims <= TileDims);
  const v3i NumBlocks(2, 2, 2);
  bitstream Bs;
  buffer Buf; AllocateBuffer(&Buf, sizeof(u64) * Prod<i64>(Dims));
  InitWrite(&Bs, Buf);
  int NumBlocksEncoded = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = IToXyz(Subbands[S].Pos, Dims);
    v3i SubbandDims = IToXyz(Subbands[S].Size, Dims);
    mg_Assert(TileDims.X <= SubbandDims.X && TileDims.Y <= SubbandDims.Y && TileDims.Z <= SubbandDims.Z);
    /* loop through the tiles within the subband */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      // TODO: can we move the following arrays to the outer scope?
      int NLevels = NumLevels(Prod<int>(BlockDims)) - 1;
      int OctreeSize = BeginIndex(NLevels);
      mg_StackArrayOfHeapArrays(Octree, u8, 8, OctreeSize);
      mg_StackArrayOfHeapArrays(PrevOctree, u8, 8, OctreeSize);
      mg_StackArrayOfHeapArrays(Block, u64, 8, Prod<int>(BlockDims));
      mg_StackArrayOfHeapArrays(BlockData, f64, 8, Prod<int>(BlockDims));
      for (int I = 0; I < 8; ++I) {
        memset(Octree[I], 0, OctreeSize * sizeof(u8));
        memset(PrevOctree[I], 0, OctreeSize * sizeof(u8));
      }
      mg_StackArrayOfHeapArrays(MsbTable, i8, 8, Prod<int>(BlockDims));
      /* loop through the bit planes */
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        /* loop through the blocks */
        for (int BI = 0; BI < Prod<int>(NumBlocks); ++BI) {
          v3i B(DecodeMorton3X(BI) * BlockDims.X + TX,
                DecodeMorton3Y(BI) * BlockDims.Y + TY,
                DecodeMorton3Z(BI) * BlockDims.Z + TZ);
          ++NumBlocksEncoded;
          if (Bitplane == Bits) { // only do this once
            int EMax = CopyBlockSamplesMorton(Data, Dims, Bits - 1, BlockDims, B, BlockData[BI], Block[BI], MsbTable[BI]);
            Write(&Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
          }
          BuildSignificanceOctree(MsbTable[BI], BlockDims, Bitplane, Octree[BI]);
          EncodeBlock(Block[BI], BlockDims, Bitplane, PrevOctree[BI], Octree[BI], &Bs, B);
          Swap(&PrevOctree[BI], &Octree[BI]);
        }
      }
    }}} // end loop through the tiles
  }
  Flush(&Bs);
  fwrite(Bs.Stream.Data, Size(&Bs), 1, Fp);
  fclose(Fp);
  printf("\n------------------------------------\n");
}

void Decode(cstr FileName, v3i Dims, v3i TileDims, int Bits, const dynamic_array<Block>& Subbands,
  f64* Data)
{
  mg_Assert(TileDims == v3i(32, 32, 32));
  v3i BlockDims(16, 16, 16);
  mg_Assert(BlockDims <= TileDims);
  const v3i NumBlocks(2, 2, 2);
  bitstream Bs;
  buffer Buf;
  ReadFile(FileName, &Buf);
  InitRead(&Bs, Buf);
  int NumBlocksDecoded = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = IToXyz(Subbands[S].Pos, Dims);
    v3i SubbandDims = IToXyz(Subbands[S].Size, Dims);
    mg_Assert(TileDims.X <= SubbandDims.X && TileDims.Y <= SubbandDims.Y && TileDims.Z <= SubbandDims.Z);
    /* loop through the tiles within the subband */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      int NLevels = NumLevels(Prod<int>(BlockDims)) - 1;
      int OctreeSize = BeginIndex(NLevels);
      mg_StackArrayOfHeapArrays(Octree, u8, 8, OctreeSize);
      mg_StackArrayOfHeapArrays(PrevOctree, u8, 8, OctreeSize);
      mg_StackArrayOfHeapArrays(Block, u64, 8, Prod<int>(BlockDims));
      for (int I = 0; I < 8; ++I) {
        memset(Octree[I], 0, OctreeSize * sizeof(u8));
        memset(PrevOctree[I], 0, OctreeSize * sizeof(u8));
        memset(Block[I], 0, Prod<int>(BlockDims) * sizeof(u64));
      }
      mg_StackArrayOfHeapArrays(MsbTable, i8, 8, Prod<int>(BlockDims));
      int EMaxes[8];
      /* loop through the blocks */
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        for (int BI = 0; BI < Prod<int>(NumBlocks); ++BI) {
          v3i B(DecodeMorton3X(BI) * BlockDims.X + TX,
                DecodeMorton3Y(BI) * BlockDims.Y + TY,
                DecodeMorton3Z(BI) * BlockDims.Z + TZ);
          ++NumBlocksDecoded;
          if (Bitplane == Bits)
            EMaxes[BI] = Read(&Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
          DecodeBlock(Block[BI], BlockDims, Bitplane, PrevOctree[BI], Octree[BI], &Bs, B);
          memcpy(PrevOctree[BI], Octree[BI], sizeof(u8) * OctreeSize); // TODO: merge this step with the below
          if (Bitplane == 0)
            CopyBlockSamplesInverseMorton(Block[BI], Dims, Bits - 1, BlockDims, B, EMaxes[BI], Data);
        }
      }
    }}} // end loop through the tiles
  }
  return;
}

/* Each tile is 32x32x32, but we encode each block of 16x16x16 independently within a tile */
void EncodeFast(const f64* Data, v3i Dims, v3i TileDims, int Bits, const dynamic_array<Block>& Subbands,
  cstr FileName)
{
  FILE* Fp = fopen(FileName, "wb");
  mg_Assert(TileDims == v3i(32, 32, 32));
  v3i BlockDims(16, 16, 16);
  mg_Assert(BlockDims <= TileDims);
  const v3i NumBlocks(2, 2, 2);
  bitstream Bs;
  buffer Buf; AllocateBuffer(&Buf, sizeof(u64) * Prod<i64>(Dims));
  InitWrite(&Bs, Buf);
  int NumBlocksEncoded = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = IToXyz(Subbands[S].Pos, Dims);
    v3i SubbandDims = IToXyz(Subbands[S].Size, Dims);
    mg_Assert(TileDims.X <= SubbandDims.X && TileDims.Y <= SubbandDims.Y && TileDims.Z <= SubbandDims.Z);
    /* loop through the tiles within the subband */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      int NLevels = NumLevels(Prod<int>(BlockDims)) - 1;
      int OctreeSize = BeginIndex(NLevels);
      mg_StackArrayOfHeapArrays(Octree, u8, 8, OctreeSize);
      mg_StackArrayOfHeapArrays(Block, u64, 8, Prod<int>(BlockDims));
      mg_StackArrayOfHeapArrays(BlockData, f64, 8, Prod<int>(BlockDims));
      mg_StackArrayOfHeapArrays(MsbTable, i8, 8, Prod<int>(BlockDims));
      /* loop through the bit planes */
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        /* loop through the blocks */
        for (int BI = 0; BI < Prod<int>(NumBlocks); ++BI) {
          v3i B(DecodeMorton3X(BI) * BlockDims.X + TX,
                DecodeMorton3Y(BI) * BlockDims.Y + TY,
                DecodeMorton3Z(BI) * BlockDims.Z + TZ);
          ++NumBlocksEncoded;
          if (Bitplane == Bits) { // only do this once
            int EMax = CopyBlockSamplesMorton(Data, Dims, Bits - 1, BlockDims, B, BlockData[BI], Block[BI], MsbTable[BI]);
            Write(&Bs, EMax + Traits<f64>::ExponentBias, Traits<f64>::ExponentBits);
          }
          if (Bitplane == Bits) {
            BuildSignificanceOctree(MsbTable[BI], BlockDims, Bitplane, Octree[BI]);
            if (Octree[BI][0] == 0) {
              Write(&Bs, 0);
            } else {
              Write(&Bs, 1);
              goto ELSE;
            }
          } else {
            ELSE:
            for (int I = 0; I < Prod<int>(BlockDims); I += 64) {
              u64 X = 0;
              for (int J = 0; J < 64; ++J)
                X += (1 & (Block[BI][I + J] >> Bitplane)) << J;
              Write(&Bs, X, 64);
              // Write(&Bs, 1 & (Block[BI][I] >> Bitplane));
            }
          }
        }
      }
    }}} // end loop through the tiles
  }
  Flush(&Bs);
  fwrite(Bs.Stream.Data, Size(&Bs), 1, Fp);
  fclose(Fp);
  printf("\n------------------------------------\n");
}

void DecodeFast(cstr FileName, v3i Dims, v3i TileDims, int Bits, const dynamic_array<Block>& Subbands,
  f64* Data)
{
  mg_Assert(TileDims == v3i(32, 32, 32));
  v3i BlockDims(16, 16, 16);
  mg_Assert(BlockDims <= TileDims);
  const v3i NumBlocks(2, 2, 2);
  bitstream Bs;
  buffer Buf;
  ReadFile(FileName, &Buf);
  InitRead(&Bs, Buf);
  int NumBlocksDecoded = 0;
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = IToXyz(Subbands[S].Pos, Dims);
    v3i SubbandDims = IToXyz(Subbands[S].Size, Dims);
    mg_Assert(TileDims.X <= SubbandDims.X && TileDims.Y <= SubbandDims.Y && TileDims.Z <= SubbandDims.Z);
    /* loop through the tiles within the subband */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      int EMaxes[8];
      mg_StackArrayOfHeapArrays(Block, u64, 8, Prod<int>(BlockDims));
      for (int I = 0; I < 8; ++I)
        memset(Block[I], 0, Prod<int>(BlockDims) * sizeof(u64));
      /* loop through the blocks */
      for (int Bitplane = Bits; Bitplane >= 0; --Bitplane) {
        for (int BI = 0; BI < Prod<int>(NumBlocks); ++BI) {
          v3i B(DecodeMorton3X(BI) * BlockDims.X + TX,
                DecodeMorton3Y(BI) * BlockDims.Y + TY,
                DecodeMorton3Z(BI) * BlockDims.Z + TZ);
          ++NumBlocksDecoded;
          if (Bitplane == Bits)
            EMaxes[BI] = Read(&Bs, Traits<f64>::ExponentBits) - Traits<f64>::ExponentBias;
          for (int I = 0; I < Prod<int>(BlockDims); ++I) {
            //u64 X = Read(&Bs, 64);
            //for (int J = 0; J < 64; ++J)
              Block[BI][I] |= Read(&Bs) << Bitplane;
          }
          if (Bitplane == 0)
            CopyBlockSamplesInverseMorton(Block[BI], Dims, Bits - 1, BlockDims, B, EMaxes[BI], Data);
        }
      }
    }}} // end loop through the tiles
  }
  return;
}

} // namespace mg