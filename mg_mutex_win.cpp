#if defined(_WIN32)

#include <windows.h>
#include "mg_mutex.h"

namespace mg {

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
