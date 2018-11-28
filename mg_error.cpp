#include <stdio.h>
#include "mg_error.h"
#include "mg_io.h"
#include "mg_types.h"

namespace mg {

error::error(error_code Code, bool StringGenerated, cstr Message)
  : Message(Message), Code(Code), StackIndex(0), StringGenerated(StringGenerated) {}

error::operator bool() const {
  return Code == error_code::NoError;
}

cstr ToString(const error& Err, bool Force) {
  if (Force || !Err.StringGenerated) {
    auto ErrStr = ToString(Err.Code);
    snprintf(ScratchBuffer, sizeof(ScratchBuffer), "%.*s (file: %s, line %d): %s",
      ErrStr.Size, ErrStr.Ptr, Err.Files[0], Err.Lines[0], Err.Message);
  }
  return ScratchBuffer;
}

void PrintStacktrace(printer* Pr, const error& Err) {
  mg_Print(Pr, "Stack trace:\n");
  for (i8 I = 0; I < Err.StackIndex; ++I) {
    mg_Print(Pr, "File %s, line %d\n", Err.Files[I], Err.Lines[I]);
  }
}

} // namespace mg
