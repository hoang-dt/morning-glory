#include <string.h>
#include "mg_args.h"
#include "mg_string.h"
#include "mg_types.h"

namespace mg {

bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, cstr* Value) {
  for (int I = 0; I + 1 < NumArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0) {
      *Value = Args[I + 1];
      return true;
    }
  }
  return false;
}

bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, int* Value) {
  for (int I = 0; I + 1 < NumArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0)
      return ToInt(Args[I + 1], Value);
  }
  return false;
}

bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, f64* Value) {
  for (int I = 0; I + 1 < NumArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0)
      return ToDouble(Args[I + 1], Value);
  }
  return false;
}

bool OptionExists(int NumArgs, cstr* Args, cstr Option) {
  for (int I = 0; I < NumArgs; ++I) {
    if (strcmp(Args[I], Option) == 0)
      return true;
  }
  return false;
}

} // namespace mg
