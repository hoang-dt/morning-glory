#pragma once

#include "mg_common_types.h"
#include "mg_dataset.h"
#include "mg_types.h"

namespace mg {

f64 SquaredError(const f64* F, const f64* G, i64 Size, data_type Type = data_type::float64);
f64 RMSError(const f64* F, const f64* G, i64 Size, data_type Type = data_type::float64);
f64 PSNR(const f64* F, const f64* G, i64 Size, data_type Type = data_type::float64);
void ConvertToNegabinary(const i64* FIn, i64 Size, u64* FOut, data_type Type = data_type::float64);
void ConvertFromNegabinary(const u64* FIn, i64 Size, i64* FOut, data_type Type = data_type::float64);
f64 DotProduct(const f64* F, const f64* G, i64 Size, data_type Type = data_type::float64);
bool CheckNaNInf(const f64* F, i64 Size, data_type Type = data_type::float64);
void Convolve(const f64* F, i64 SizeF, const f64* G, int ng, f64* H, data_type Type = data_type::float64);
void UpsampleZeros(const f64* FIn, i64 Size, f64* FOut, data_type Type = data_type::float64);
f64 SquareNorm(const f64* F, i64 Size, data_type Type = data_type::float64);
f64 Norm(const f64* F, i64 Size, data_type Type = data_type::float64);
int Quantize(const f64* FIn, i64 Size, int Bits, i64* FOut, data_type Type = data_type::float64);
void Dequantize(const i64* FIn, i64 Size, int EMax, int Bits, f64* FOut, data_type Type = data_type::float64);

} // namespace mg
