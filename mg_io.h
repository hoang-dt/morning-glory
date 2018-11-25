#pragma once

#include "mg_error.h"
#include "mg_types.h"

namespace mg {

/* Print formatted strings into a buffer */
struct printer {
  char* Buf = nullptr;
  int Size = 0;
  printer() = default;
  printer(char* Buf, int Size);
};

void Reset(printer* Pr, char* Buf, int Size);
// template <typename ... args>
// void Print(printer* Pr, cstr Fmt, args&... Args);
#define mg_PrintFmt(PrinterPtr, Format, ...)
#define mg_Print(PrinterPtr, Message)

/* Read a text file from disk into a buffer. The buffer can be nullptr or it can be
initialized in advance, in which case the existing memory will be reused if the file can fit
in it. The caller is responsible to deallocate the memory. */
error ReadFile(cstr Fname, buffer* Buf);

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

#include "mg_io.inl"
