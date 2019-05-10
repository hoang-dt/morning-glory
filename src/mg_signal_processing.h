#pragma once

#include "mg_common.h"

namespace mg {

// TODO: take a volume
struct grid_volume;
f64  SqError(const grid_volume& F, const grid_volume& G);
f64  RMSError(const grid_volume& F, const grid_volume& G);
f64  PSNR(const grid_volume& F, const grid_volume& G);
void FwdNegaBinary(const grid_volume& Src, grid_volume* Dst);
void InvNegaBinary(const grid_volume& Src, grid_volume* Dst);
int  Quantize(const grid_volume& Src, grid_volume* Dst, int Bits);
void Dequantize(const grid_volume& Src, int EMax, int Bits, grid_volume* Dst);

} // namespace mg
