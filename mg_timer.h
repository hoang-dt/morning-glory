#pragma once

#include "mg_types.h"

namespace mg {
struct timer;
void StartTimer(timer* Timer);
i64 ElapsedTime(timer* Timer);
i64 ResetTimer(timer* Timer);
} // namespace mg

#include "mg_timer.inl"
