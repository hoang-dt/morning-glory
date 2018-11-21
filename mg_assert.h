#pragma once

#include <stdarg.h>
#include <stdio.h>
#include "mg_debugbreak.h"

#if defined(mg_Slow)
  #define mg_Assert(Cond) \
    do { \
      if (!(Cond)) { \
        fprintf(stderr, "Condition %s failed, ", #Cond); \
        fprintf(stderr, "in file %s, line %d\n", __FILE__, __LINE__); \
        debug_break(); \
      } \
    } while (0)
  #define mg_AssertMsg(Cond, Msg, ...) \
    do { \
      if (!(Cond)) { \
        fprintf(stderr, "Condition %s failed, ", #Cond); \
        fprintf(stderr, "in file %s, line %d\n", __FILE__, __LINE__); \
        va_list Args; \
        va_start(Args, Msg); \
        vfprintf(stderr, Msg, Args); \
        va_end(Args); \
        debug_break(); \
      } \
    } while (0)
#else
  #define mg_Assert(Cond) ;
  #define mg_AssertMsg(Cond, Msg) ;
#endif
