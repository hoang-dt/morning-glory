#pragma once

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace mg {

struct mutex {
  CRITICAL_SECTION Crit;
  mutex();
  ~mutex();
};

struct lock {
  mutex* Mutex;
  lock(mutex* Mutex);
  ~lock();
};

} // namespace mg

#endif
