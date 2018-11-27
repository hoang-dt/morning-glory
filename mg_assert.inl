#pragma once

// TODO: write these into functions so we can better deal with circular includes
// TODO: add a mg_Fatal macro or something that always fires regardless of mg_Slow

#include <stdio.h>
#include "mg_debugbreak.h"
#include "mg_io.h"
#include "mg_stacktrace.h"
#include "mg_macros.h"

#undef mg_Assert
#undef mg_AssertMsg
#undef mg_AssertFmt

#if defined(mg_Slow)
  #define mg_Assert(Cond) \
    do { \
      if (!(Cond)) { \
        fprintf(stderr, "Condition \"%s\" failed, ", #Cond); \
        fprintf(stderr, "in file %s, line %d\n", __FILE__, __LINE__); \
        mg::printer Pr(stderr);\
        mg::PrintStacktrace(&Pr);\
        debug_break(); \
      } \
    } while (0)
  #define mg_AssertMsg(Cond, Msg) \
    do { \
      if (!(Cond)) { \
        fprintf(stderr, "Condition \"%s\" failed, ", #Cond); \
        fprintf(stderr, "in file %s, line %d:", __FILE__, __LINE__); \
        fprintf(stderr, Msg);\
        fprintf(stderr, "\n");\
        mg::printer Pr(stderr);\
        mg::PrintStacktrace(&Pr);\
        debug_break(); \
      } \
    } while (0)
  #define mg_AssertFmt(Cond, Fmt, ...) \
    do { \
      if (!(Cond)) { \
        fprintf(stderr, "Condition \"%s\" failed, ", #Cond); \
        fprintf(stderr, "in file %s, line %d:", __FILE__, __LINE__); \
        fprintf(stderr, Fmt, __VA_ARGS__);\
        fprintf(stderr, "\n");\
        mg::printer Pr(stderr);\
        mg::PrintStacktrace(&Pr);\
        debug_break(); \
      } \
    } while (0)
#else
  #define mg_Assert(Cond) do {} while (0)
  #define mg_AssertMsg(Cond, Msg) do {} while (0)
  #define mg_AssertFmt(Cond, Fmt, ...) do {} while (0)
#endif
