#pragma once

#include "mg_macros.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace mg {
struct mutex {
  CRITICAL_SECTION Crit;
  mutex();
  ~mutex();
};

struct lock {
  mutex* Mutex = nullptr;
  lock(mutex* MutexIn);
  ~lock();
};

mg_Inline mutex::
mutex() { InitializeCriticalSection(&Crit); }

mg_Inline mutex::
~mutex() { DeleteCriticalSection(&Crit); }

mg_Inline lock::
lock(mutex* MutexIn) : Mutex(MutexIn) { Lock(Mutex); }

mg_Inline lock::
~lock() { Unlock(Mutex); }

mg_Inline bool
Lock(mutex* Mutex) {
  EnterCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

mg_Inline bool
TryLock(mutex* Mutex) { return TryEnterCriticalSection(&Mutex->Crit) != 0; }

mg_Inline bool
Unlock(mutex* Mutex) {
  LeaveCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

} // namespace mg
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
namespace mg {
struct mutex {
  pthread_mutex_t Mx;
};

struct lock {
  mutex* Mutex = nullptr;
  lock(mutex* Mutex);
  ~lock();
};

mg_Inline lock::
lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }

mg_Inline lock::
~lock() { Unlock(Mutex); }

mg_Inline bool
Lock(mutex* Mutex) { return pthread_mutex_lock(&Mutex->Mx) == 0; }

mg_Inline bool
TryLock(mutex* Mutex) { return pthread_mutex_trylock(&Mutex->Mx) == 0; }

mg_Inline bool
Unlock(mutex* Mutex) { return pthread_mutex_unlock(&Mutex->Mx) == 0; }

} // namespace mg
#endif

