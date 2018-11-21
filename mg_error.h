#pragma once

#include "mg_types.h"

//mg_Enum2(errorcode2, int, NoError1, Error2)

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
  inline thread_local static char FullMessage[256];
  explicit operator bool();
}; // struct error

cstr ToString(error& Err);

#define mg_Error(Code) mg::error{ __FILE__, "", mg::int16(__LINE__), mg::error_code::Code }
#define mg_ErrorMsg(Code, Message) mg::error{ __FILE__, Message, mg::int16(__LINE__), mg::error_code::Code }

}
