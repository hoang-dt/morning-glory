#pragma once

#if defined(__linux__) || defined(__APPLE__)

#include <pthread.h>

namespace mg {

struct mutex {
  pthread_mutex_t Mx;
};

}

#endif
