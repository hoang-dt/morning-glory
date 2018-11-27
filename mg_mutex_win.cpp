#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "mg_mutex.h"

namespace mg {

mutex::mutex() { InitializeCriticalSection(&Crit); }
mutex::~mutex() { DeleteCriticalSection(&Crit); }

lock::lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }
lock::~lock() { Unlock(Mutex); }

bool Lock(mutex* Mutex) {
  EnterCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

bool TryLock(mutex* Mutex) {
  return TryEnterCriticalSection(&Mutex->Crit) != 0;
}

bool Unlock(mutex* Mutex) {
  LeaveCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

} // namespace mg

#endif
