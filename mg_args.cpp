#include "mg_args.h"

#include <string.h>

namespace mg {

cstr GetOptionValue(int NumArgs, cstr* Args, cstr Option) {
  for (int I = 0; I + 1 < NumArgs; ++I) {
    if (strncmp(Args[I], Option, 32) == 0)
      return Args[I + 1];
  }
  return nullptr;
}

bool OptionExists(int NumArgs, cstr* Args, cstr Option) {
  for (int I = 0; I < NumArgs; ++I) {
    if (strcmp(Args[I], Option) == 0)
      return true;
  }
  return false;
}

} // namespace mg
