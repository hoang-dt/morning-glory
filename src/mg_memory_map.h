#pragma once

#include "mg_types.h"
#include "mg_bitops.h"
#include "mg_enum.h"
#include "mg_error.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

// TODO: create a mapping that is not backed by a file

mg_Enum(mmap_err_code, int, mg_CommonErrs,
  MappingFailed, MapViewFailed, FlushFailed, SyncFailed, UnmapFailed)

namespace mg {

enum class map_mode { Read, Write };

#if defined(_WIN32)
using file_handle = HANDLE;
#elif defined(__linux__) || defined(__APPLE__)
using file_handle = int;
#endif

struct mmap_file {
  file_handle File;
  file_handle FileMapping;
  map_mode Mode;
  buffer Buf;
};

error<mmap_err_code> open_file(const char* Name, map_mode Mode, mmap_file* MMap);
error<mmap_err_code> map_file(mmap_file* MMap, i64 Bytes = 0);
error<mmap_err_code> flush_file(mmap_file* MMap, byte* Start = nullptr, i64 Bytes = 0);
error<mmap_err_code> sync_file(mmap_file* MMap);
error<mmap_err_code> unmap_file(mmap_file* MMap);
error<mmap_err_code> close_file(mmap_file* MMap);

} // namespace mg
