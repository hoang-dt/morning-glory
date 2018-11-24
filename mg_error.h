#pragma once

#include "mg_enum.h"
#include "mg_types.h"

mg_Enum(error_code, int,
  NoError,
  SizeZero, SizeTooSmall, SizeMismatched,
  DimensionMismatched, DimensionsTooMany,
  AttributeNotFound,
  OptionNotSupported,
  TypeNotSupported,
  FileCreateFailed, FileReadFailed, FileWriteFailed, FileOpenFailed, FileCloseFailed, FileSeekFailed, FileTellFailed,
  ParseFailed,
  OutOfMemory,
  UnknownError
)

namespace mg {

struct error {
  cstr File = "";
  cstr Message = "";
  error_code Code = error_code::NoError;
  int16 Line = 0;
  bool StringGenerated = false;
  inline thread_local static char FullMessage[256]; /* There should be only one error in-flight on each thread */
  explicit operator bool();
}; // struct error

cstr ToString(error& Err, bool Force = false);

} // namespace mg

#define mg_Error(ErrCode)
#define mg_ErrorMsg(ErrCode, Msg)
#define mg_ErrorFmt(ErrCode, Fmt, ...)

#include "mg_error.inl"
