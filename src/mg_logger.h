#pragma once

// TODO: add logging levels
// TODO: multithreading?
// TODO: per-file buffering mode
// TODO: do we need more than one logger?

#include "mg_common.h"

#define mg_MaxSlots 16

namespace mg {

enum class buffer_mode { Full, Line, None };

struct logger {
  stack_array<FILE*, mg_MaxSlots> FHandles    = {};
  stack_array<cstr , mg_MaxSlots> FNames      = {};
  stack_array<u32  , mg_MaxSlots> FNameHashes = {};
  buffer_mode Mode = buffer_mode::Full;
};


inline logger GlobalLogger;

void SetBufferMode(logger* Logger, buffer_mode Mode);
void SetBufferMode(buffer_mode Mode); // set buffer mode for the global logger

} // namespace mg

/*
Use the following macro for logging as follows
mg_Log("log.txt", "Message %d", A)
mg_Log(stderr, "Message %f", B) */
#define mg_Log(FileName, Format, ...)

#include "mg_logger.inl"
