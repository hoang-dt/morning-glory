#include "mg_array.h"
#include "mg_assert.h"
#include "mg_math.h"
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
  mg_Assert(IsPow2(NumValues));
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

// TODO: remove the last octree level (since it's unnecessary)
void BuildSignificanceOctree(const i8* MsbTable, v3i BlockDims, int Bitplane, u8* Octree) {
  mg_Assert(BlockDims.X == 16 && BlockDims.Y == 16 && BlockDims.Z == 16);
  int NLevels = NumLevels(Prod<int>(BlockDims)) - 1;
  mg_Assert(NLevels == 4); // assuming 16^3 blocks
  for (int L = NLevels - 1; L > 0; --L) {
    int Begin = BeginIndex(L);
    int End = EndIndex(L);
    if (L == NLevels - 1) { // last level
      for (int I = Begin; I < End; ++I)
        Octree[I] = (((u8(MsbTable[I * 8 + 0] >= Bitplane) << 0)   |
                      (u8(MsbTable[I * 8 + 1] >= Bitplane) << 1))  |
                     ((u8(MsbTable[I * 8 + 2] >= Bitplane) << 2)   |
                      (u8(MsbTable[I * 8 + 3] >= Bitplane) << 3))) |
                    (((u8(MsbTable[I * 8 + 4] >= Bitplane) << 4)   |
                      (u8(MsbTable[I * 8 + 5] >= Bitplane) << 5))  |
                     ((u8(MsbTable[I * 8 + 6] >= Bitplane) << 6)   |
                      (u8(MsbTable[I * 8 + 7] >= Bitplane) << 7)));
    }
    for (int I = Begin; I < End; I += 8) {
      int ParentIdx = ParentIndex(I);
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

/* Each tile is 32x32x32, but we encode each block of 16x16x16 independently within a tile */
void Encode(const f64* Data, v3i Dims, v3i TileDims, const dynamic_array<Block>& Subbands,
  cstr FileName)
{
  v3i BlockDims{ 16, 16, 16 };
  mg_Assert(BlockDims.X <= TileDims.X && BlockDims.Y < TileDims.Y && BlockDims.Z <= TileDims.Z);
  /*  */
  for (int S = 0; S < Size(Subbands); ++S) {
    v3i SubbandPos = IToXyz(Subbands[S].Pos, Dims);
    v3i SubbandDims = IToXyz(Subbands[S].Size, Dims);
    mg_Assert(TileDims.X <= SubbandDims.X && TileDims.Y <= SubbandDims.Y && TileDims.Z <= SubbandDims.Z);
    /* loop through the tiles */
    for (int TZ = SubbandPos.Z; TZ < SubbandPos.Z + SubbandDims.Z; TZ += TileDims.Z) {
    for (int TY = SubbandPos.Y; TY < SubbandPos.Y + SubbandDims.Y; TY += TileDims.Y) {
    for (int TX = SubbandPos.X; TX < SubbandPos.X + SubbandDims.X; TX += TileDims.X) {
      /* loop through the blocks */
      for (int BZ = 0; BZ < TileDims.Z; BZ += BlockDims.Z) {
      for (int BY = 0; BY < TileDims.Y; BY += BlockDims.Y) {
      for (int BX = 0; BX < TileDims.X; BX += BlockDims.X) {
        // TODO: handle partial blocks

      }}}
    }}} // end loop through the tiles
  }
}

} // namespace mg