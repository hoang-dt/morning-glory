#pragma once

#include <assert.h>
#include <string.h>
#include "mg_macros.h"
#include "mg_memory.h"

#define mg_SPrintHelper(...)\
  __VA_OPT__(snprintf(ScratchBuffer + L, sizeof(ScratchBuffer) - L, __VA_ARGS__)); L

#define mg_ExtractFirst(X, ...) X

#undef mg_Error
#define mg_Error(ErrCode, ...)\
  [&]() {\
    if (mg_NumArgs(__VA_ARGS__) > 0) {\
      mg::error Err{ mg::error_code::ErrCode, true __VA_OPT__(,) mg_ExtractFirst(__VA_ARGS__) };\
      Err.Files[0] = __FILE__;\
      Err.Lines[0] = __LINE__;\
      Err.Code = mg::error_code::ErrCode;\
      auto ErrStr = ToString(Err.Code);\
      int L = snprintf(ScratchBuffer, sizeof(ScratchBuffer), "%.*s (file %s, line %d): ",\
        ErrStr.Size, ErrStr.Ptr, __FILE__, __LINE__);\
      mg_SPrintHelper(__VA_ARGS__);\
      return Err;\
    }\
    mg::error Err{ mg::error_code::ErrCode };\
    Err.Files[0] = __FILE__;\
    Err.Lines[0] = __LINE__;\
    return Err;\
  }();

#undef mg_PropagateError
#define mg_PropagateError(Err)\
  [&Err]() {\
    if (Err.StackIndex >= 64)\
      assert(false && "stack too deep");\
    ++Err.StackIndex;\
    Err.Lines[Err.StackIndex] = __LINE__;\
    Err.Files[Err.StackIndex] = __FILE__;\
    return Err;\
  }();
