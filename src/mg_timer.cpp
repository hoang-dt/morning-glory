// TODO: forceinline these functions?
#include "mg_timer.h"

namespace mg {

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void StartTimer(timer* Timer) {
  LARGE_INTEGER Li;
  QueryPerformanceCounter(&Li);
  Timer->CounterStart = Li.QuadPart;
}
// TODO: take a const reference
i64 ElapsedTime(timer* Timer) {
  LARGE_INTEGER Li;
  QueryPerformanceCounter(&Li);
  return (Li.QuadPart - Timer->CounterStart) / Timer->PCFreq;
}
#elif defined(__linux__) || defined(__APPLE__)
void StartTimer(timer* Timer) {
	clock_gettime(CLOCK_MONOTONIC, &Timer->Start);
}
i64 ElapsedTime(timer* Timer) {
  timespec End;
	clock_gettime(CLOCK_MONOTONIC, &End);
  return 1000 * (End.tv_sec - Timer->Start.tv_sec) + (End.tv_nsec - Timer->Start.tv_nsec) / 1e6;
}
#endif

i64 ResetTimer(timer* Timer) {
  i64 Elapsed = ElapsedTime(Timer);
  StartTimer(Timer);
  return Elapsed;
}

} // namespace mg
