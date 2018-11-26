#pragma once

#include <assert.h>
#include <stdio.h>

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

#undef mg_PrintFmt
#define mg_PrintFmt(PrinterPtr, Format, ...) {\
  if ((PrinterPtr)->Buf && !(PrinterPtr)->File) {\
    int Written = snprintf((PrinterPtr)->Buf, (PrinterPtr)->Size, Format, __VA_ARGS__);\
    (PrinterPtr)->Buf += Written;\
    (PrinterPtr)->Size += Written;\
  } else if (!(PrinterPtr)->Buf && (PrinterPtr)->File) {\
    fprintf((PrinterPtr)->File, Format, __VA_ARGS__);\
  }\
}
#undef mg_Print
#define mg_Print(PrinterPtr, Message) {\
  if ((PrinterPtr)->Buf && !(PrinterPtr)->File) {\
    int Written = snprintf((PrinterPtr)->Buf, (PrinterPtr)->Size, Message);\
    (PrinterPtr)->Buf += Written;\
    (PrinterPtr)->Size += Written;\
  } else if (!(PrinterPtr)->Buf && (PrinterPtr)->File) {\
    fprintf((PrinterPtr)->File, Message);\
  } else {\
    assert(false && "unavailable or ambiguous printer destination");\
  }\
}

// template <typename ... args>
// void Print(printer* Pr, cstr Fmt, args&... Args) {
//   int Written = snprintf(Pr->Buf, Pr->Size, Fmt, Args...);
//   Pr->Buf += Written;
//   Pr->Size -= Written;
// }

} // namespace mg
