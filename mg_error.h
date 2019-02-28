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
  FileCreateFailed, FileReadFailed, FileWriteFailed, FileOpenFailed, FileCloseFailed,
  FileSeekFailed, FileTellFailed,
  ParseFailed,
  OutOfMemory,
  UnknownError
)

namespace mg {

/* There should be only one error in-flight on each thread */
struct error {
  cstr Message = "";
  error_code Code = error_code::NoError;
  i8 StackIndex = 0;
  bool StringGenerated = false;
  error(error_code Code, bool StringGenerated = false, cstr Message = "");
  inline thread_local static cstr Files[64]; // Store file names up the stack
  inline thread_local static i16 Lines[64]; // Store line numbers up the stack
  explicit operator bool() const; // Return true if no error
}; // struct error

cstr ToString(const error& Err, bool Force = false);
struct printer;
void PrintStacktrace(printer* Pr, const error& Err);

} // namespace mg

#define mg_Error(ErrCode, ...)
// Use this to record file and line information in Error when propagating it up the stack
#define mg_PropagateError(Error) 

#include "mg_error.inl"
