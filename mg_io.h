#pragma once

#include <stdio.h>
#include "mg_error.h"
#include "mg_types.h"

#define mg_FSeek
#define mg_FTell

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

/* Read a text file from disk into a buffer. The buffer can be nullptr or it 
 * can be initialized in advance, in which case the existing memory will be 
 * reused if the file can fit in it. The caller is responsible to deallocate
 * the memory. */
error<> ReadFile(cstr FileName, buffer* Buf);
error<> WriteFile(cstr FileName, const buffer& Buf);

} // namespace mg

#include "mg_io.inl"
