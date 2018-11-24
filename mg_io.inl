#pragma once

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
