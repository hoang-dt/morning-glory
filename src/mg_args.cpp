#include <string.h>
#include "mg_args.h"
#include "mg_string.h"

namespace mg {

bool 
OptVal(int NArgs, cstr* Args, cstr Opt, cstr* Val) {
  for (int I = 0; I + 1 < NArgs; ++I) {
    if (strncmp(Args[I], Opt, 32) == 0) {
      *Val = Args[I + 1];
      return true;
    }
  }
  return false;
}

bool 
OptVal(int NArgs, cstr* Args, cstr Opt, int* Val) {
  for (int I = 0; I + 1 < NArgs; ++I) {
    if (strncmp(Args[I], Opt, 32) == 0)
      return ToInt(Args[I + 1], Val);
  }
  return false;
}

bool
OptVal(int NArgs, cstr* Args, cstr Opt, v3i* Val) {
  for (int I = 0; I + 3 < NArgs; ++I) {
    if (strncmp(Args[I], Opt, 32) == 0) {
      return ToInt(Args[I + 1], &Val->X) &&
             ToInt(Args[I + 2], &Val->Y) &&
             ToInt(Args[I + 3], &Val->Z);
    }
  }
  return false;
}

bool 
OptVal(int NArgs, cstr* Args, cstr Opt, f64* Val) {
  for (int I = 0; I + 1 < NArgs; ++I) {
    if (strncmp(Args[I], Opt, 32) == 0)
      return ToDouble(Args[I + 1], Val);
  }
  return false;
}

bool 
OptExists(int NArgs, cstr* Args, cstr Opt) {
  for (int I = 0; I < NArgs; ++I) {
    if (strcmp(Args[I], Opt) == 0)
      return true;
  }
  return false;
}

} // namespace mg
