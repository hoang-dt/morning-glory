#pragma once

#if defined(_WIN32)

#include <windows.h>

namespace mg {

struct mutex {
  CRITICAL_SECTION Crit;
  mutex() { InitializeCriticalSection(&Crit); }
  ~mutex() { DeleteCriticalSection(&Crit); }
};

} // namespace mg

#endif
