#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

struct grid;
struct volume;

/* Computing errors between two functions */
mg_TT(t1, t2) f64
SqError(const buffer_t<t1>& FBuf, const buffer_t<t2>& GBuf);
mg_TT(t1, t2) f64
SqError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
f64
SqError(const volume& FVol, const volume& GVol);

mg_TT(t1, t2) f64
RMSError(const buffer_t<t1>& FBuf, const buffer_t<t2>& GBuf);
mg_TT(t1, t2) f64
RMSError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
f64
RMSError(const volume& FVol, const volume& GVol);

mg_TT(t1, t2) f64
PSNR(const buffer_t<t1>& FBuf, const buffer_t<t2>& GBuf);
mg_TT(t1, t2) f64
PSNR(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
f64
PSNR(const volume& FVol, const volume& GVol);

/* Negabinary */
mg_TT(t, u) void
FwdNegaBinary(const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_TT(t1, t2) void
FwdNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
void
FwdNegaBinary(const volume& SVol, volume* DVol);

mg_TT(t, u) void
InvNegaBinary(const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_TT(t1, t2) void
InvNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
void
InvNegaBinary(const volume& SVol, volume* DVol);

/* Quantization */
mg_TT(t, u) int
Quantize(int Bits, const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_TT(t1, t2) int
Quantize(int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
int
Quantize(int Bits, const volume& SVol, volume* DVol);

mg_TT(t, u) void
Dequantize(int EMax, int Bits, const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_TT(t1, t2) void
Dequantize(int EMax, int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
void
Dequantize(int EMax, int Bits, const volume& SVol, volume* DVol);

/* Convert the type */
mg_TT(t1, t2) void
ConvertType(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
void
ConvertType(const volume& SVol, volume* DVol);

mg_T(t) f64 Norm(const t& Begin, const t& End);

mg_T(c) void Upsample(const c& In, c* Out);

mg_T(c) void Convolve(const c& F, const c& G, c* H);

} // namespace mg

