#include "mg_array.h"
#include "mg_assert.h"
#include "mg_wavelet.h"
#include "mg_types.h"

namespace mg {

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