#pragma once

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mg_macros.h"
#include "mg_memory.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_types.h"

namespace mg {

template <typename t>
error<t>::error(t ErrCode, bool StrGenerated, cstr Msg) : 
  Msg(Msg), ErrCode(ErrCode), StackIdx(0), 
  StrGenerated(StrGenerated) {}

template <typename t>
cstr ToString(const error<t>& Err, bool Force) {
  if (Force || !Err.StrGenerated) {
    auto ErrStr = ToString(Err.ErrCode);
    snprintf(ScratchBuf, sizeof(ScratchBuf), "%.*s (file: %s, line %d): %s",
      ErrStr.Size, ErrStr.Ptr, Err.Files[0], Err.Lines[0], Err.Msg);
  }
  return ScratchBuf;
}

template <typename t>
void PrintStacktrace(printer* Pr, const error<t>& Err) {
  mg_Print(Pr, "Stack trace:\n");
  for (i8 I = 0; I < Err.StackIdx; ++I)
    mg_Print(Pr, "File %s, line %d\n", Err.Files[I], Err.Lines[I]);
}

} // namespace mg

#define mg_TempSprintHelper(...)\
  __VA_OPT__(snprintf(ScratchBuf + L, sizeof(ScratchBuf) - L, __VA_ARGS__));\
  mg_Unused(L)

#define mg_ExtractFirst(X, ...) X

#undef mg_Error
#define mg_Error(ErrorCode, ...)\
  [&]() {\
    if (mg_NumArgs(__VA_ARGS__) > 0) {\
      mg::error Err(ErrorCode, true __VA_OPT__(,) mg_ExtractFirst(__VA_ARGS__));\
      Err.Files[0] = __FILE__;\
      Err.Lines[0] = __LINE__;\
      auto ErrStr = ToString(Err.ErrCode);\
      int L = snprintf(\
        ScratchBuf, sizeof(ScratchBuf), "%.*s (file %s, line %d): ",\
        ErrStr.Size, ErrStr.Ptr, __FILE__, __LINE__);\
      mg_TempSprintHelper(__VA_ARGS__);\
      return Err;\
    }\
    mg::error Err(ErrorCode);\
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
