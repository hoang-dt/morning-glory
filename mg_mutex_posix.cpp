#include "mg_mutex.h"

#if defined(__linux__) || defined(__APPLE__)

namespace mg {

lock::lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }
lock::~lock() { Unlock(Mutex); }

bool Lock(mutex* Mutex) {
  return pthread_mutex_lock(&Mutex->Mx) == 0;
}

bool TryLock(mutex* Mutex) {
  return pthread_mutex_trylock(&Mutex->Mx) == 0;
}

bool Unlock(mutex* Mutex) {
  return pthread_mutex_unlock(&Mutex->Mx) == 0;
}

} // namespace mg

#endif
