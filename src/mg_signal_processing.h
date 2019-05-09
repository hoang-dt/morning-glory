#pragma once

#include "mg_common.h"
#include "mg_volume.h"

namespace mg {

// TODO: take a volume

f64  SqError(const grid<volume>& F, const grid<volume>& G);
f64  RMSError(const grid<volume>& F, const grid<volume>& G);
f64  PSNR(const grid<volume>& F, const grid<volume>& G);
void ToNegaBinary(grid<volume>* Dst, const grid<volume>& Src);
void FromNegaBinary(grid<volume>* Dst, const grid<volume>& Src);
int  Quantize(grid<volume>* Dst, const grid<volume>& Src, int Bits);
void Dequantize(byte* FIn, i64 Size, int EMax, int Bits, byte* FOut, dtype Type = dtype::float64);

} // namespace mg
