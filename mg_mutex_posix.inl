#pragma once

#if defined(__linux__) || defined(__APPLE__)

#include <pthread.h>

namespace mg {

struct mutex {
  pthread_mutex_t Mx;
};

struct lock {
  mutex* Mutex;
  lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }
  ~lock() { Unlock(Mutex); }
};

} // namespace mg

#endif
