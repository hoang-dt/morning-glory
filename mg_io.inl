#pragma once

#include <assert.h>
#include <stdio.h>

// TODO: remove one macro, instead use ## to paste __VA_ARGS__

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

#undef mg_Print
#define mg_Print(PrinterPtr, Format, ...) {\
  if ((PrinterPtr)->Buf && !(PrinterPtr)->File) {\
    if ((PrinterPtr)->Size <= 1)\
      assert(false && "buffer too small"); /* TODO: always abort */ \
    int Written = snprintf((PrinterPtr)->Buf, (PrinterPtr)->Size, Format, ##__VA_ARGS__);\
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
