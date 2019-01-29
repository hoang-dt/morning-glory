#pragma once

#include "mg_common_types.h"
#include "mg_dataset.h"
#include "mg_types.h"

namespace mg {

f64 SquaredError(const byte* F, const byte* G, i64 Size, data_type Type = data_type::float64);
f64 RMSError(const byte* F, const byte* G, i64 Size, data_type Type = data_type::float64);
f64 PSNR(const byte* F, const byte* G, i64 Size, data_type Type = data_type::float64);
void ConvertToNegabinary(const byte* FIn, i64 Size, byte* FOut, data_type Type = data_type::int64);
void ConvertFromNegabinary(const byte* FIn, i64 Size, byte* FOut, data_type Type = data_type::int64);
f64 DotProduct(const byte* F, const byte* G, i64 Size, data_type Type = data_type::float64);
bool CheckNaNInf(const byte* F, i64 Size, data_type Type = data_type::float64);
void Convolve(const byte* F, i64 SizeF, const byte* G, int ng, byte* H, data_type Type = data_type::float64);
void UpsampleZeros(const byte* FIn, i64 Size, byte* FOut, data_type Type = data_type::float64);
f64 SquareNorm(const byte* F, i64 Size, data_type Type = data_type::float64);
f64 Norm(const byte* F, i64 Size, data_type Type = data_type::float64);
int Quantize(const byte* FIn, i64 Size, int Bits, byte* FOut, data_type Type = data_type::float64);
void Dequantize(const byte* FIn, i64 Size, int EMax, int Bits, byte* FOut, data_type Type = data_type::float64);

} // namespace mg
