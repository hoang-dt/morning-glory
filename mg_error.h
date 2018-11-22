#pragma once

#include <string.h>
#include "mg_types.h"

namespace mg {

enum error_code {
  NoError,
  SizeTooSmall, SizeMismatched,
  DimensionMismatched,
  OptionNotSupported,
  TypeUnsupported,
  FileCreateFailed, FileReadFailed, FileWriteFailed, FileOpenFailed, FileCloseFailed, FileSeekFailed, FileTellFailed,
  ParseFailed,
  OutOfMemory,
  UnknownError,
  NumErrorCodes
};

constexpr cstr ErrorStr[] = {
  "NoError",
  "SizeTooSmall", "SizeMismatched",
  "DimensionMismatched",
  "OptionNotSupported",
  "TypeUnsupported",
  "FileCreateFailed", "FileReadFailed", "FileWriteFailed", "FileOpenFailed", "FileCloseFailed", "FileSeekFailed", "FileTellFailed",
  "ParseFailed",
  "OutOfMemory",
  "UnknownError"
};
static_assert(sizeof(ErrorStr)/sizeof(ErrorStr[0]) == NumErrorCodes);

struct error {
  cstr File = "";
  cstr Message = "";
  int16 Line = 0;
  error_code Code = NoError;
  bool StringGenerated = false;
  inline thread_local static char FullMessage[256]; /* There should be only one error in-flight on each thread */
  explicit operator bool();
}; // struct error

cstr ToString(error& Err, bool Force = false);

#define mg_Error(ErrCode)\
  mg::error{ __FILE__, "", mg::i16(__LINE__), mg::error_code::ErrCode, false }
#define mg_ErrorMsg(ErrCode, Msg)\
  mg::error{ __FILE__, Msg, mg::int16(__LINE__), mg::error_code::ErrCode }
#define mg_ErrorFmt(ErrCode, Fmt, ...)\
  [&]() {\
    mg::error Err{ __FILE__, Fmt, mg::i16(__LINE__), mg::error_code::ErrCode, true };\
    Err.Code = mg::error_code::ErrCode;\
    auto ErrStr = mg::ErrorStr[Err.Code];\
    snprintf(Err.FullMessage, sizeof(Err.FullMessage), "%s (file %s, line %d): ",\
      ErrStr, __FILE__, __LINE__);\
    auto L = strlen(Err.FullMessage);\
    snprintf(Err.FullMessage + L, sizeof(Err.FullMessage) - L, Fmt, __VA_ARGS__);\
    return Err;\
  }();

} // namespace mg
