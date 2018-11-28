#pragma once

#include <stdio.h>
#include "mg_debugbreak.h"
#include "mg_io.h"
#include "mg_stacktrace.h"
#include "mg_macros.h"

#define mg_FPrintHelper(...)\
  __VA_OPT__(fprintf(stderr, __VA_ARGS__))

#define mg_AssertHelper(Cond, ...)\
  do {\
    if (!(Cond)) {\
      fprintf(stderr, "Condition \"%s\" failed, ", #Cond);\
      fprintf(stderr, "in file %s, line %d\n", __FILE__, __LINE__);\
      if (mg_NumArgs(__VA_ARGS__) > 0) {\
        mg_FPrintHelper(__VA_ARGS__);\
        fprintf(stderr, "\n");\
      }\
      mg::printer Pr(stderr);\
      mg::PrintStacktrace(&Pr);\
      debug_break();\
    }\
  } while (0);

#undef mg_Assert
#if defined(mg_Slow)
  #define mg_Assert(Cond, ...) mg_AssertHelper(Cond, __VA_ARGS__)
#else
  #define mg_Assert(Cond, ...) do {} while (0)
#endif

#undef mg_AbortIf
#define mg_AbortIf(Cond, ...) mg_AssertHelper(Cond && "Fatal error!", __VA_ARGS__)
#undef mg_Abort
#define mg_Abort(...) mg_AbortIf(!"Fatal error!", __VA_ARGS__)
