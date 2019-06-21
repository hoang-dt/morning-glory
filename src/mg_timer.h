#pragma once

#include "mg_common.h"

namespace mg {

struct timer;
void StartTimer (timer* Timer);
i64  ElapsedTime(timer* Timer); // return nanoseconds
i64  ResetTimer (timer* Timer); // return nanoseconds
f64  Milliseconds(i64 Nanosecs);
f64  Seconds(i64 Nanosecs);

} // namespace mg

#include "mg_timer.inl"
