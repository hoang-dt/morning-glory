#pragma once

#include "mg_error_codes.h"
#include "mg_types.h"

namespace mg {

/* There should be only one error in-flight on each thread */
template <typename t = err_code>
struct error {
  cstr Msg = "";
  t Code;
  i8 StackIdx = 0;
  bool StrGenerated = false;
  error();
  error(t Code, bool StrGenerated = false, cstr Msg = "");
  inline thread_local static cstr Files[64]; // Store file names up the stack
  inline thread_local static i16 Lines[64]; // Store line numbers up the stack
}; // struct err_template

template <typename t>
cstr ToString(const error<t>& Err, bool Force = false);

struct printer;
template <typename t>
void PrintStacktrace(printer* Pr, const error<t>& Err);

template <typename t>
bool ErrorExists(const error<t>& Err);

} // namespace mg

#define mg_Error(ErrCode, ...)
// Use this to record file and line information in Error when propagating it up the stack
#define mg_PropagateError(Error)

#include "mg_error.inl"
