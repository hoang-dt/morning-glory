#pragma once

// TODO: add logging levels
// TODO: multithreading?
// TODO: per-file buffering mode
// TODO: do we need more than one logger?

namespace mg {

enum class buffer_mode { Full, Line, None };
struct logger;

logger& GlobalLogger();
void SetBufferMode(logger* Logger, buffer_mode Mode);
void SetBufferMode(buffer_mode Mode); // set buffer mode for the global logger

} // namespace mg

/* Use the following macro for logging as follows
mg_Log("log.txt", "Message %d", A)
mg_Log(stderr, "Message %f", B) */
#define mg_Log(FileName, Format, ...)

#include "mg_logger.inl"
