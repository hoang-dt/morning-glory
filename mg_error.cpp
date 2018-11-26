#include <stdio.h>
#include "mg_error.h"
#include "mg_io.h"
#include "mg_types.h"

namespace mg {

error::operator bool() {
  return Code == error_code::NoError;
}

cstr ToString(error& Err, bool Force) {
  if (Force || !Err.StringGenerated) {
    auto ErrStr = ToString(Err.Code);
    snprintf(Err.FullMessage, sizeof(Err.FullMessage), "%.*s (file: %s, line %d): %s",
      ErrStr.Size, ErrStr.Ptr, Err.Files[0], Err.Lines[0], Err.Message);
  }
  return Err.FullMessage;
}

void PrintStacktrace(printer* Pr, error& Err) {
  mg_Print(Pr, "Stack trace:\n");
  for (i8 I = 0; I < Err.StackIndex; ++I) {
    mg_PrintFmt(Pr, "File %s, line %d\n", Err.Files[I], Err.Lines[I]);
  }
}

} // namespace mg
