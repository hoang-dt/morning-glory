#include "mg_mutex.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace mg {

mutex::
mutex() { InitializeCriticalSection(&Crit); }

mutex::
~mutex() { DeleteCriticalSection(&Crit); }

lock::
lock(mutex* MutexIn) : Mutex(MutexIn) { Lock(Mutex); }

lock::
~lock() { Unlock(Mutex); }

bool
Lock(mutex* Mutex) {
  EnterCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

bool
TryLock(mutex* Mutex) {
  return TryEnterCriticalSection(&Mutex->Crit) != 0;
}

bool
Unlock(mutex* Mutex) {
  LeaveCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

} // namespace mg
#elif defined(__linux__) || defined(__APPLE__)
namespace mg {
lock::
lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }

lock::
~lock() { Unlock(Mutex); }

bool
Lock(mutex* Mutex) {
  return pthread_mutex_lock(&Mutex->Mx) == 0;
}

bool
TryLock(mutex* Mutex) {
  return pthread_mutex_trylock(&Mutex->Mx) == 0;
}

bool
Unlock(mutex* Mutex) {
  return pthread_mutex_unlock(&Mutex->Mx) == 0;
}

} // namespace mg
#endif
