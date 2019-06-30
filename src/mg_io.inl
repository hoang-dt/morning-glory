#pragma once

#include <assert.h>
#include <stdio.h>
#include "mg_scopeguard.h"

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

mg_T(i) error<>
DumpText(cstr FileName, i Begin, i End, cstr Format) {
  FILE* Fp = fopen(FileName, "w");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  if (!Fp)
    return mg_Error(err_code::FileCreateFailed, "%s", FileName);
  for (i It = Begin; It != End; ++It) {
    if (fprintf(Fp, Format, *It) < 0)
      return mg_Error(err_code::FileWriteFailed);
  }
  return mg_Error(err_code::NoError);
}

} // namespace mg
