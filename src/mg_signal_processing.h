#pragma once

#include "mg_data_types.h"
#include "mg_common.h"
#include "mg_volume.h"

namespace mg {

// TODO: take a volume

f64
SquaredError(byte* F, byte* G, i64 Size, data_type Type = data_type::float64);

f64
SquaredError(volume& F, volume& G);

f64
RMSError(byte* F, byte* G, i64 Size, data_type Type = data_type::float64);

f64
RMSError(volume& F, volume& G);

f64
PSNR(byte* F, byte* G, i64 Size, data_type Type = data_type::float64);

f64
PSNR(volume& F, volume& G);

void
ConvertToNegabinary(byte* FIn, i64 Size, byte* FOut, data_type Type = data_type::int64);

void
ConvertFromNegabinary(byte* FIn, i64 Size, byte* FOut, data_type Type = data_type::int64);

f64
DotProduct(byte* F, byte* G, i64 Size, data_type Type = data_type::float64);

bool
CheckNaNInf(byte* F, i64 Size, data_type Type = data_type::float64);

void
Convolve(byte* F, i64 SizeF, byte* G, int ng, byte* H, data_type Type = data_type::float64);

void
UpsampleZeros(byte* FIn, i64 Size, byte* FOut, data_type Type = data_type::float64);

f64
SquareNorm(byte* F, i64 Size, data_type Type = data_type::float64);

f64
Norm(byte* F, i64 Size, data_type Type = data_type::float64);

int
Quantize(byte* FIn, i64 Size, int Bits, byte* FOut, data_type Type = data_type::float64);

void
Dequantize(byte* FIn, i64 Size, int EMax, int Bits, byte* FOut, data_type Type = data_type::float64);

} // namespace mg
