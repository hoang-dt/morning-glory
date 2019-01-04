#pragma once

#include <stdio.h>
#include "mg_error.h"
#include "mg_types.h"

#define mg_FSeek
#define mg_FTell

// TODO: write a logger

namespace mg {

/* Print formatted strings into a buffer */
struct printer {
  char* Buf = nullptr;
  int Size = 0;
  FILE* File = nullptr; // either File == nullptr or Buf == nullptr
  printer();
  printer(char* Buf, int Size);
  printer(FILE* File);
};

void Reset(printer* Pr, char* Buf, int Size);
void Reset(printer* Pr, FILE* File);
#define mg_Print(PrinterPtr, Format, ...)

/* Read a text file from disk into a buffer. The buffer can be nullptr or it can be
initialized in advance, in which case the existing memory will be reused if the file can fit
in it. The caller is responsible to deallocate the memory. */
error ReadFile(cstr FileName, buffer* Buf);
error WriteFile(cstr FileName, const buffer& Buf);

// /* Write a list of values to a text file */
// template <typename T>
// Expected<bool> write_values(const char* file_name, const T* values, int64_t n, bool append=false) {
//   thread_local static char buf[65536];
//   std::ofstream fs;
//   fs.rdbuf()->pubsetbuf(buf, sizeof(buf));
//   fs.open(file_name, append ? std::ios::app : std::ios::trunc);
//   if (!fs)
//     return mg_ErrorFmt(FileCreateFailed);
//   fs << n << "\n";
//   for (int64_t i = 0; i < n; ++i)
//     fs << values[i] << "\n";
//   if (!fs)
//     return mg_ErrorFmt(FileWriteFailed);
//   fs.close();
//   if (!fs)
//     return mg_ErrorFmt(FileCloseFailed);
//   return true;
// }

} // namespace mg

#include "mg_io.inl"
