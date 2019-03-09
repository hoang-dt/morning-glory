#pragma once

#include "mg_enum.h"
#include "mg_types.h"

// TODO: spread the common errors across multiple modules

#define mg_CommonErrs\
  NoError, UnknownError,\
  SizeZero, SizeTooSmall, SizeMismatched,\
  DimensionMismatched, DimensionsTooMany,\
  AttributeNotFound,\
  OptionNotSupported,\
  TypeNotSupported,\
  FileCreateFailed, FileReadFailed, FileWriteFailed, FileOpenFailed,\
  FileCloseFailed, FileSeekFailed, FileTellFailed,\
  ParseFailed,\
  OutOfMemory

mg_Enum(err_code, int, mg_CommonErrs)

namespace mg {

/* There should be only one error in-flight on each thread */
template <typename t = err_code>
struct error {  
  cstr Msg = "";
  t ErrCode;
  i8 StackIdx = 0;
  bool StrGenerated = false;
  error(t ErrCode, bool StrGenerated = false, cstr Msg = "");
  inline thread_local static cstr Files[64]; // Store file names up the stack
  inline thread_local static i16 Lines[64]; // Store line numbers up the stack
  explicit operator bool() const; // Return true if no error
}; // struct err_template

template <typename t> 
cstr ToString(const error<t>& Err, bool Force = false);
struct printer;
template <typename t> 
void PrintStacktrace(printer* Pr, const error<t>& Err);

} // namespace mg

#define mg_Error(ErrCode, ...)
// Use this to record file and line information in Error when propagating it up the stack
#define mg_PropagateError(Error) 

#include "mg_error.inl"
