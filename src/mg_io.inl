#pragma once

#include <assert.h>
#include <stdio.h>

#undef mg_FSeek
#undef mg_FTell
/* Enable support for reading large files */
#if defined(_WIN32)
  #define mg_FSeek _fseeki64
  #define mg_FTell _ftelli64
#elif defined(__linux__) || defined(__APPLE__)
  #define _FILE_OFFSET_BITS 64
  #define mg_FSeek fseeko
  #define mg_FTell ftello
#endif

namespace mg {

#undef mg_Print
#define mg_Print(PrinterPtr, Format, ...) {\
  if ((PrinterPtr)->Buf && !(PrinterPtr)->File) {\
    if ((PrinterPtr)->Size <= 1)\
      assert(false && "buffer too small"); /* TODO: always abort */ \
    int Written = snprintf((PrinterPtr)->Buf, size_t((PrinterPtr)->Size),\
                           Format, ##__VA_ARGS__);\
    (PrinterPtr)->Buf += Written;\
    if (Written < (PrinterPtr)->Size)\
      (PrinterPtr)->Size -= Written;\
    else\
      assert(false && "buffer overflow?");\
  } else if (!(PrinterPtr)->Buf && (PrinterPtr)->File) {\
    fprintf((PrinterPtr)->File, Format, ##__VA_ARGS__);\
  } else {\
    assert(false && "unavailable or ambiguous printer destination");\
  }\
}

} // namespace mg
