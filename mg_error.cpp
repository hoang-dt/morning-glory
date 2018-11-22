#include "mg_error.h"
#include "mg_types.h"

namespace mg {

error::operator bool() {
  return Code == error_code::NoError;
}

cstr ToString(error& Err, bool Force) {
  if (Force || !Err.StringGenerated) {
    auto ErrStr = ToString(Err.Code);
    snprintf(Err.FullMessage, sizeof(Err.FullMessage), "%.*s (file: %s, line %d): %s",
      ErrStr.Size, ErrStr.Ptr, Err.File, Err.Line, Err.Message);
  }
  return Err.FullMessage;
}

} // namespace mg
