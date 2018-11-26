#pragma once

#if defined(_WIN32)

#include <windows.h>

namespace mg {

struct mutex {
  CRITICAL_SECTION Crit;
  mutex() { InitializeCriticalSection(&Crit); }
  ~mutex() { DeleteCriticalSection(&Crit); }
};

struct lock {
  mutex* Mutex;
  lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }
  ~lock() { Unlock(Mutex); }
};

} // namespace mg

#endif
