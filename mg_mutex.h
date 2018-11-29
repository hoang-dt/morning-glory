#pragma once

namespace mg {

struct mutex;
struct lock; // RAII struct that acquires the mutex on construction and releases it on destruction

/* Block until the lock can be acquired then return true. Return false if something goes wrong.  */
bool Lock(mutex* Mutex);
/* Return immediately: true if the lock can be acquired, false if not */
bool TryLock(mutex* Mutex);
/* Return true if the lock can be released. Return false if something goes wrong. */
bool Unlock(mutex* Mutex);

}

#include "mg_mutex.inl"
