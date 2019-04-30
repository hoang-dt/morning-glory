#pragma once

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "mg_types.h"
namespace mg {
struct timer {
  inline const static i64 PCFreq = []() {
    LARGE_INTEGER Li;
    bool Ok = QueryPerformanceFrequency(&Li);
    return Ok ? Li.QuadPart / 1000 : 0;
  }();
  i64 CounterStart = 0;
};
}
#elif defined(__linux__) || defined(__APPLE__)
#include <time.h>
namespace mg {
struct timer {
  timespec Start;
};
}
#endif
