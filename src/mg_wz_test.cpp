#include "mg_array.h"
#include "mg_common.h"
#include "mg_file_format.h"
#include "mg_test.h"
#include "mg_wavelet.h"

/*
This function is called whenever all the data in a tile8

A tile8 corresponds to 8 tiles in 3D (4 in 2D): if a tile is 32^3, a tile8 is 64^3,
consisting of eight corresponding tiles in 8 subbands on the same level. When we
do wavelet transform on a tile8, it produces the 8 tiles one on each subband.

- Count stores one number from 0 to 7 for each level, indicating the current tile
- LastTile = true if this function is triggered by the last tile in the previous
- Tiles stores one tile8 for each level */
//void
//WriteTile8(grid* Tile8, const array<i8>& Count, const file_format& Wz,
//           array<grid> Tile8s, int Level, bool LastTile)
//{
  ///* perform wavelet transform on the current tile8 */
  //ForwardCdf53(Tile8, 1); // TODO: normalize the coefficients
  ///* separate the eight subbands */
  //grid_volume[8] Tiles; // each grid corresponds to a tile
  //for (int I = 0; I < 8; ++I) {
  //  Tiles[I] = grid_volume(Wz.TileDims + 1, Wz.DType); // TODO: add DType
  //}
  //array<grid> Subbands;
  //BuildSubbandsInPlace(Dims(*Tile8), 1, &Subbands); // TODO: take into account extrapolation
  //for (int S = 0; S < Size(Subbands); ++S) {
  //  Copy();
  //}
  ///* encode the last seven tiles */
  //for (int S = 1; S < Size(Subbands); ++S) {

  //}
  ///* encode the first tile too if this is the last level */
  //// TODO
  ///* write the compressed tiles into a file if enough tiles have been processed
  //in the current level */
  //// NOTE: we interleave the tiles but not across levels
  //// TODO
  ///* if this is the last tile (x = y = z = 1), write the tile in the next level */
  //// TODO
//}
