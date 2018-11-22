#pragma once

#include <stdio.h>
#include "mg_error.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_types.h"

/* Enable support for reading large files */
#if defined(_MSC_VER) || defined(__MINGW64__)
  #define mg_FSeek _fseeki64
  #define mg_FTell _ftelli64
#elif defined(__GNUC__) || defined(__APPLE__)
  #define _FILE_OFFSET_BITS 64
  #define mg_FSeek fseeko
  #define mg_FTell ftello
#endif

namespace mg {

/* Read a text file from disk into a string */
error ReadFile(cstr Fname, buffer* Buf) {
  mg_Assert((Buf->Data && Buf->Size) || (!Buf->Data && !Buf->Size));

  FILE* Fp = fopen(Fname, "rb");
  mg_BeginCleanUp(0) { if (Fp) fclose(Fp); }; mg_EndCleanUp(0)
  if (!Fp) { return mg_ErrorMsg(FileOpenFailed, Fname); }

  /* Determine the file size */
  if (mg_FSeek(Fp, 0, SEEK_END)) return mg_ErrorMsg(FileSeekFailed, Fname);
  size_t Size = 0;
  if ((Size = mg_FTell(Fp)) == size_t(-1)) return mg_ErrorMsg(FileTellFailed, Fname);
  if (mg_FSeek(Fp, 0, SEEK_SET)) return mg_ErrorMsg(FileSeekFailed, Fname);
  if (Buf->Size < Size)
    if (!mg_Allocate(Buf->Data, Size)) return mg_ErrorMsg(OutOfMemory, Fname);

  /* Read file contents */
  mg_BeginCleanUp(1) { mg_Deallocate(Buf->Data); }; mg_EndCleanUp(1)
  if (fread(Buf->Data, Size, 1, Fp) != 1) return mg_ErrorMsg(FileReadFailed, Fname);
  Buf->Size = Size;

  mg_DismissCleanUp(1);
  return mg_Error(NoError);
}

/* Write a binary raw file to disk */
// Error write_raw(const char* file_name, const byte* buf, int64_t size) {
//   FILE* fp = fopen(file_name,"wb");
//   if (!fp) {
//     if (fp) fclose(fp);
//     return mg_Error(FileNotFound);
//   }
//   if (fwrite(buf, size, 1, fp) != 1) {
//     if (fp) fclose(fp);
//     return mg_Error(FileWriteFailed);
//   }
//   if (fp) {
//     if (fclose(fp))
//       return mg_Error(FileCloseFailed);
//   }
//   return true;
// }

// /* Write a list of values to a text file */
// template <typename T>
// Expected<bool> write_values(const char* file_name, const T* values, int64_t n, bool append=false) {
//   thread_local static char buf[65536];
//   std::ofstream fs;
//   fs.rdbuf()->pubsetbuf(buf, sizeof(buf));
//   fs.open(file_name, append ? std::ios::app : std::ios::trunc);
//   if (!fs)
//     return mg_Error(FileCreateFailed);
//   fs << n << "\n";
//   for (int64_t i = 0; i < n; ++i)
//     fs << values[i] << "\n";
//   if (!fs)
//     return mg_Error(FileWriteFailed);
//   fs.close();
//   if (!fs)
//     return mg_Error(FileCloseFailed);
//   return true;
// }

} // namespace mg
