#include "mg_assert.h"
#include "mg_math.h"

namespace mg {

int Pow(int Base, int Exp) {
  mg_Assert(Exp >= 0);
  int Result = 1;
  for (int I = 0; I < Exp; ++I)
    Result *= Base;
  return Result;
}

int GeometricSum(int Base, int N) {
  mg_Assert(N >= 0);
  return (Pow(Base, N + 1) - 1) / (Base - 1);
}

} // namespace mg
