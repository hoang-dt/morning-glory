#pragma once

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mg_memory.h"

namespace mg {

mg_T(t) error<t>::
error() {}

mg_T(t) error<t>::
error(t CodeIn, bool StrGenedIn, cstr MsgIn) :
  Msg(MsgIn), Code(CodeIn), StackIdx(0), StrGened(StrGenedIn) {}

mg_T(t) error<t>::
operator bool() const {
  return Code == t::NoError;
}

mg_T(t) cstr
ToString(const error<t>& Err, bool Force) {
  if (Force || !Err.StrGened) {
    auto ErrStr = ToString(Err.Code);
    snprintf(ScratchBuf, sizeof(ScratchBuf), "%.*s (file: %s, line %d): %s",
             ErrStr.Size, ErrStr.Ptr, Err.Files[0], Err.Lines[0], Err.Msg);
  }
  return ScratchBuf;
}

mg_T(t) void
PrintStacktrace(printer* Pr, const error<t>& Err) {
  mg_Print(Pr, "Stack trace:\n");
  for (i8 I = 0; I < Err.StackIdx; ++I)
    mg_Print(Pr, "File %s, line %d\n", Err.Files[I], Err.Lines[I]);
}

mg_T(t) bool
ErrorExists(const error<t>& Err) { return Err.Code != t::NoError; }

} // namespace mg

#undef mg_Error
#define mg_Error(ErrCode, ...)\
  [&]() {\
    if (mg_NumArgs(__VA_ARGS__) > 0) {\
      mg::error Err(ErrCode, true __VA_OPT__(,) mg_ExtractFirst(__VA_ARGS__));\
      Err.Files[0] = __FILE__;\
      Err.Lines[0] = __LINE__;\
      auto ErrStr = ToString(Err.Code);\
      int L = snprintf(ScratchBuf, sizeof(ScratchBuf), "%.*s (file %s, line %d): ",\
                       ErrStr.Size, ErrStr.Ptr, __FILE__, __LINE__);\
      mg_SPrintHelper(ScratchBuf, L, __VA_ARGS__);\
      return Err;\
    }\
    mg::error Err(ErrCode);\
    Err.Files[0] = __FILE__;\
    Err.Lines[0] = __LINE__;\
    return Err;\
  }();

#undef mg_PropagateError
#define mg_PropagateError(Err)\
  [&Err]() {\
    if (Err.StackIdx >= 64)\
      assert(false && "stack too deep");\
    ++Err.StackIdx;\
    Err.Lines[Err.StackIdx] = __LINE__;\
    Err.Files[Err.StackIdx] = __FILE__;\
    return Err;\
  }();
