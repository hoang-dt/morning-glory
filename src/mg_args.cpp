#include <string.h>
#include "mg_args.h"
#include "mg_string.h"

namespace mg {

bool OptionValue(int NArgs, cstr* Args, cstr Option, cstr* Value) {
  for (int I = 0; I + 1 < NArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0) {
      *Value = Args[I + 1];
      return true;
    }
  }
  return false;
}

bool OptionValue(int NArgs, cstr* Args, cstr Option, int* Value) {
  for (int I = 0; I + 1 < NArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0)
      return ToInt(Args[I + 1], Value);
  }
  return false;
}

bool OptionValue(int NArgs, cstr* Args, cstr Option, v3i* Value) {
  for (int I = 0; I + 3 < NArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0) {
      return ToInt(Args[I + 1], &Value->X) &&
             ToInt(Args[I + 2], &Value->Y) &&
             ToInt(Args[I + 3], &Value->Z);
    }
  }
  return false;
}

bool OptionValue(int NArgs, cstr* Args, cstr Option, f64* Value) {
  for (int I = 0; I + 1 < NArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0)
      return ToDouble(Args[I + 1], Value);
  }
  return false;
}

bool OptionExists(int NArgs, cstr* Args, cstr Option) {
  for (int I = 0; I < NArgs; ++I) {
    if (strcmp(Args[I], Option) == 0)
      return true;
  }
  return false;
}

} // namespace mg
