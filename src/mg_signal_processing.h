#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

struct grid;
struct volume;

/* Computing errors between two functions */
mg_T(t) f64
SqError(const buffer_t<t>& FBuf, const buffer_t<t>& GBuf);
mg_T2(t1, t2) f64
SqError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
mg_T(t) f64
RMSError(const buffer_t<t>& FBuf, const buffer_t<t>& GBuf);
mg_T2(t1, t2) f64
RMSError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
mg_T(t) f64
PSNR(const buffer_t<t>& FBuf, const buffer_t<t>& GBuf);
mg_T2(t1, t2) f64
PSNR(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);

/* Negabinary */
mg_T2(t, u) void
FwdNegaBinary(const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_T2(t1, t2) void
FwdNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
mg_T2(t, u) void
InvNegaBinary(const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_T2(t1, t2) void
InvNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);

/* Quantization */
mg_T2(t, u) int
Quantize(int Bits, const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_T2(t1, t2) int
Quantize(int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
mg_T2(t, u) void
Dequantize(int EMax, int Bits, const buffer_t<t>& SBuf, buffer_t<u>* DBuf);
mg_T2(t1, t2) void
Dequantize(int EMax, int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);

} // namespace mg
