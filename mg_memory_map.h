#pragma once

#include "mg_types.h"
#include "mg_bitops.h"
#include "mg_enum.h"
#include "mg_error.h"
#include "mg_expected.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#elif defined(__linux__) || defined(__APPLE__)
#endif

// TODO: create a mapping that is not backed by a file

mg_Enum(mmap_err_code, int, mg_CommonErrs,
  MappingFailed, MapViewFailed, FlushFailed, SyncFailed, UnmapFailed)

namespace mg {

enum class map_mode { Read, Write, ReadWrite };

#if defined(_WIN32)
using file_handle = HANDLE;
#elif defined(__linux__) || defined(__APPLE__)
#endif

struct mmap_file {
#if defined(_WIN32)
  HANDLE File;
  HANDLE FileMapping;
#endif
  map_mode Mode;
  buffer Buf;
};

/* Size is only used when Mode is Write or ReadWrite */
error<mmap_err_code> open_file(const char* Name, map_mode Mode, mmap_file* MMap)
{
#if defined(_WIN32)
  HANDLE File =
    CreateFileA(Name,
                Mode == map_mode::Read ? GENERIC_READ
                          : GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
  if (File == INVALID_HANDLE_VALUE)
    return mg_Error(mmap_err_code::FileCreateFailed);
  MMap->File = File;
  MMap->Mode = Mode;
  return mg_Error(mmap_err_code::NoError);
#endif
}

error<mmap_err_code> map_file(mmap_file* MMap, u64 Bytes = 0) {
#if defined(_WIN32)
  LARGE_INTEGER FileSize{0, 0};
  if (!GetFileSizeEx(MMap->File, &FileSize) || Bytes != 0)
    FileSize.QuadPart = Bytes;
  MMap->FileMapping =
    CreateFileMapping(MMap->File,
                      NULL,
                      MMap->Mode == map_mode::Read ? PAGE_READONLY
                                                   : PAGE_READWRITE,
                      FileSize.HighPart,
                      FileSize.LowPart,
                      0);
  if (MMap->FileMapping == NULL)
    return mg_Error(mmap_err_code::MappingFailed);

  LPVOID MapAddress =
    MapViewOfFile(MMap->FileMapping,
                  MMap->Mode == map_mode::Read ? FILE_MAP_READ
                    : MMap->Mode == map_mode::Write ? FILE_MAP_WRITE
                      : FILE_MAP_ALL_ACCESS,
                  0,
                  0,
                  0);
  if (MapAddress == NULL)
    return mg_Error(mmap_err_code::MapViewFailed);
  MMap->Buf.Data = (byte*)MapAddress;
  MMap->Buf.Bytes = FileSize.QuadPart;
  return mg_Error(mmap_err_code::NoError);
#endif
}

/* (Non-blocking) flush dirty pages */
error<mmap_err_code> flush_file(mmap_file* MMap, u64 Bytes = 0) {
#if defined(_WIN32)
  if (!FlushViewOfFile(MMap->Buf.Data, Bytes))
    return mg_Error(mmap_err_code::FlushFailed);
  return mg_Error(mmap_err_code::NoError);
#endif
}

/* (Blocking) flush file metadata and ensure file is physically written */
error<mmap_err_code> sync_file(mmap_file* MMap) {
#if defined(_WIN32)
  if (!FlushFileBuffers(MMap->File))
    return mg_Error(mmap_err_code::SyncFailed);
  return mg_Error(mmap_err_code::NoError);
#endif
}

/* Unmap the file and close all handles */
error<mmap_err_code> unmap_file(mmap_file* MMap) {
#if defined(_WIN32)
  if (!UnmapViewOfFile(MMap->Buf.Data))
    return mg_Error(mmap_err_code::UnmapFailed);
  if (!CloseHandle(MMap->FileMapping))
    return mg_Error(mmap_err_code::UnmapFailed);
  return mg_Error(mmap_err_code::NoError);
#endif
}

/* Close the file */
error<mmap_err_code> close_file(mmap_file* MMap) {
#if defined(_WIN32)
  if (!CloseHandle(MMap->File))
    return mg_Error(mmap_err_code::FileCloseFailed);
  return mg_Error(mmap_err_code::NoError);
#endif
}

} // namespace mg
