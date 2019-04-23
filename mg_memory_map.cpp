#include "mg_memory_map.h"

namespace mg {

/* Size is only used when Mode is Write or ReadWrite */
error<mmap_err_code> open_file(const char* Name, map_mode Mode, mmap_file* MMap)
{
#if defined(_WIN32)
  MMap->File =
    CreateFileA(Name,
                Mode == map_mode::Read ? GENERIC_READ
                          : GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
  if (MMap->File == INVALID_HANDLE_VALUE)
    return mg_Error(mmap_err_code::FileCreateFailed);
#elif defined(__linux__) || defined(__APPLE__)
  MMap->File = open(Name, Mode == map_mode::Read ? O_RDONLY : O_RDWR);
  if (MMap->File == -1)
    return mg_Error(mmap_err_code::FileCreateFailed);
#endif
  MMap->Mode = Mode;
  return mg_Error(mmap_err_code::NoError);
}

error<mmap_err_code> map_file(mmap_file* MMap, i64 Bytes) {
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
#elif defined(__linux__) || defined(__APPLE__)
  size_t FileSize;
  struct stat Stat;
  if (Bytes != 0)
    FileSize = Bytes;
  else if (fstat(MMap->File, &Stat) == 0)
    FileSize = Stat.st_size;
  void* MapAddress =
    mmap(0,
         FileSize,
         MMap->Mode == map_mode::Read ? PROT_READ : PROT_WRITE,
         MAP_SHARED,
         MMap->File,
         0);
  if (MapAddress == MAP_FAILED)
    return mg_Error(mmap_err_code::MapViewFailed);
  MMap->Buf.Data = (byte*)MapAddress;
  MMap->Buf.Bytes = FileSize;
#endif
  return mg_Error(mmap_err_code::NoError);
}

/* (Non-blocking) flush dirty pages */
error<mmap_err_code> flush_file(mmap_file* MMap, byte* Start, i64 Bytes) {
#if defined(_WIN32)
  bool Result = Start ? FlushViewOfFile(Start, Bytes)
                      : FlushViewOfFile(MMap->Buf.Data, Bytes);
  if (!Result)
    return mg_Error(mmap_err_code::FlushFailed);
#elif defined(__linux__) || defined(__APPLE__)
  int Result;
  if (Start) {
    mg_Assert(MMap->Buf.Data <= Start && Start < MMap->Buf.Data + MMap->Buf.Bytes);
    mg_Assert(Bytes <= (MMap->Buf.Data + MMap->Buf.Bytes) - Start);
    Result = Bytes ? msync(Start, Bytes, MS_ASYNC)
                   : msync(Start, MMap->Buf.Bytes - (Start - MMap->Buf.Data), MS_ASYNC);
  } else {
    mg_Assert(Bytes <= MMap->Buf.Bytes);
    Result = Bytes ? msync(MMap->Buf.Data, Bytes, MS_ASYNC)
                   : msync(MMap->Buf.Data, MMap->Buf.Bytes, MS_ASYNC);
  }
  if (Result == -1)
    return mg_Error(mmap_err_code::FlushFailed);
#endif
  return mg_Error(mmap_err_code::NoError);
}

/* (Blocking) flush file metadata and ensure file is physically written.
 * Meant to be called at the very end */
error<mmap_err_code> sync_file(mmap_file* MMap) {
#if defined(_WIN32)
  if (!FlushFileBuffers(MMap->File))
    return mg_Error(mmap_err_code::SyncFailed);
#elif defined(__linux__) || defined(__APPLE__)
  if (msync(MMap->Buf.Data, MMap->Buf.Bytes, MS_SYNC) == -1)
    return mg_Error(mmap_err_code::SyncFailed);
#endif
  return mg_Error(mmap_err_code::NoError);
}

/* Unmap the file and close all handles */
error<mmap_err_code> unmap_file(mmap_file* MMap) {
#if defined(_WIN32)
  if (!UnmapViewOfFile(MMap->Buf.Data)) {
    CloseHandle(MMap->FileMapping);
    return mg_Error(mmap_err_code::UnmapFailed);
  }
  if (!CloseHandle(MMap->FileMapping))
    return mg_Error(mmap_err_code::UnmapFailed);
#elif defined(__linux__) || defined(__APPLE__)
  if (munmap(MMap->Buf.Data, MMap->Buf.Bytes) == -1)
    return mg_Error(mmap_err_code::UnmapFailed);
#endif
  return mg_Error(mmap_err_code::NoError);
}

/* Close the file */
error<mmap_err_code> close_file(mmap_file* MMap) {
#if defined(_WIN32)
  if (!CloseHandle(MMap->File))
    return mg_Error(mmap_err_code::FileCloseFailed);
#elif defined(__linux__) || defined(__APPLE__)
  if (close(MMap->File) == -1)
    return mg_Error(mmap_err_code::FileCloseFailed);
#endif
  return mg_Error(mmap_err_code::NoError);
}

} // namespace mg

