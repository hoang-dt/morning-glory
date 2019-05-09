#pragma once

#include "mg_macros.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace mg {

struct timer {
  inline const static i64 PCFreq = []() {
    LARGE_INTEGER Li;
    bool Ok = QueryPerformanceFrequency(&Li);
    return Ok ? Li.QuadPart / 1000 : 0;
  }();
  i64 CounterStart = 0;
};

mg_Inline void
StartTimer(timer* Timer) {
  LARGE_INTEGER Li;
  QueryPerformanceCounter(&Li);
  Timer->CounterStart = Li.QuadPart;
}

// TODO: take a const reference
mg_Inline i64
ElapsedTime(timer* Timer) {
  LARGE_INTEGER Li;
  QueryPerformanceCounter(&Li);
  return (Li.QuadPart - Timer->CounterStart) / Timer->PCFreq;
}

} // namespace mg
#elif defined(__linux__) || defined(__APPLE__)
#include <time.h>

namespace mg {

struct timer {
  timespec Start;
};

mg_Inline void
StartTimer(timer* Timer) {
  clock_gettime(CLOCK_MONOTONIC, &Timer->Start);
}

mg_Inline i64
ElapsedTime(timer* Timer) {
  timespec End;
  clock_gettime(CLOCK_MONOTONIC, &End);
  return 1000 * (End.tv_sec - Timer->Start.tv_sec) + (End.tv_nsec - Timer->Start.tv_nsec) / 1e6;


} // namespace mg
#endif

namespace mg {

i64
ResetTimer(timer* Timer) {
  i64 Elapsed = ElapsedTime(Timer);
  StartTimer(Timer);
  return Elapsed;
}

} // namespace mg

