#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

struct grid;
struct volume;

mg_T2(t1, t2) f64
SqError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
mg_T2(t1, t2) f64
RMSError(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
mg_T2(t1, t2) f64
PSNR(const t1& FGrid, const volume& FVol, const t2& GGrid, const volume& GVol);
mg_T2(t1, t2) void
FwdNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
mg_T2(t1, t2) void
InvNegaBinary(const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
mg_T2(t1, t2) int
Quantize(int Bits, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);
mg_T2(t1, t2) void
Dequantize(int Bits, int EMax, const t1& SGrid, const volume& SVol, const t2& DGrid, volume* DVol);

} // namespace mg
