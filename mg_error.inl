#pragma once

#include <string.h>

#undef mg_Error
#define mg_Error(ErrCode)\
  mg::error{ __FILE__, "", mg::error_code::ErrCode, mg::i16(__LINE__), false }

#undef mg_ErrorMsg
#define mg_ErrorMsg(ErrCode, Msg)\
  mg::error{ __FILE__, Msg, mg::error_code::ErrCode, mg::int16(__LINE__) }

#undef mg_ErrorFmt
#define mg_ErrorFmt(ErrCode, Fmt, ...)\
  [&]() {\
    mg::error Err{ __FILE__, Fmt, mg::error_code::ErrCode, mg::i16(__LINE__), true };\
    Err.Code = mg::error_code::ErrCode;\
    auto ErrStr = ToString(Err.Code);\
    snprintf(Err.FullMessage, sizeof(Err.FullMessage), "%.*s (file %s, line %d): ",\
      ErrStr.Size, ErrStr.Ptr, __FILE__, __LINE__);\
    auto L = strlen(Err.FullMessage);\
    snprintf(Err.FullMessage + L, sizeof(Err.FullMessage) - L, Fmt, __VA_ARGS__);\
    return Err;\
  }();
