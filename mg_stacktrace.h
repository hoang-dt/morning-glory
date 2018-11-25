#pragma once

#if defined(_WIN32)
#elif defined __linux__ || defined __APPLE__
#include "mg_stacktrace_posix.h"
#else
#error Platform not supported
#endif
