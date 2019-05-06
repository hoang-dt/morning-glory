#include "mg_assert.h"

namespace mg {

int
Pow(int Base, int Exp) {
  mg_Assert(Exp >= 0);
  int Result = 1;
  for (int I = 0; I < Exp; ++I)
    Result *= Base;
  return Result;
}

} // namespace mg
