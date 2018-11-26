#pragma once

#include <assert.h>
#include <string.h>

#undef mg_Error
#define mg_Error(ErrCode)\
  []() {\
    mg::error Err{ "", mg::error_code::ErrCode, 0, false };\
    Err.Files[0] = __FILE__;\
    Err.Lines[0] = __LINE__;\
    return Err;\
  }();

#undef mg_ErrorMsg
#define mg_ErrorMsg(ErrCode, Msg)\
  [&]() {\
    mg::error Err{ Msg, mg::error_code::ErrCode, 0, false };\
    Err.Files[0] = __FILE__;\
    Err.Lines[0] = __LINE__;\
    return Err;\
  }();

#undef mg_ErrorFmt
#define mg_ErrorFmt(ErrCode, Fmt, ...)\
  [&]() {\
    mg::error Err{ Fmt, mg::error_code::ErrCode, 0, true };\
    Err.Files[0] = __FILE__;\
    Err.Lines[0] = __LINE__;\
    Err.Code = mg::error_code::ErrCode;\
    auto ErrStr = ToString(Err.Code);\
    snprintf(Err.FullMessage, sizeof(Err.FullMessage), "%.*s (file %s, line %d): ",\
      ErrStr.Size, ErrStr.Ptr, __FILE__, __LINE__);\
    auto L = strlen(Err.FullMessage);\
    snprintf(Err.FullMessage + L, sizeof(Err.FullMessage) - L, Fmt, __VA_ARGS__);\
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
