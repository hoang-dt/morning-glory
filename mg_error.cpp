#include "mg_error.h"
#include "mg_types.h"

namespace mg {

error::operator bool() {
  return Code == error_code::NoError;
}

cstr ToString(error& Err) {
  snprintf(Err.FullMessage, sizeof(Err.FullMessage), "%s: %s (file: %s, line %d)",
    ErrorStr[Err.Code], Err.Message, Err.File, Err.Line);
  return Err.FullMessage;
}

} // namespace mg
