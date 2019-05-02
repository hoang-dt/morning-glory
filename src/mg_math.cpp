#include "mg_assert.h"
#include "mg_bitops.h"
#include "mg_math.h"
#include "mg_types.h"

namespace mg {

int Pow(int Base, int Exp) {
  mg_Assert(Exp >= 0);
  int Result = 1;
  for (int I = 0; I < Exp; ++I)
    Result *= Base;
  return Result;
}

// TODO: when n is already a power of two plus one, do not increase n
int NextPow2(int Val) {
  mg_Assert(Val >= 0);
  if (Val == 0)
    return 1;
  return 1 << (Msb((u32)(Val - 1)) + 1);
}

int GeometricSum(int Base, int N) {
  mg_Assert(N >= 0);
  return (Pow(Base, N + 1) - 1) / (Base - 1);
}

} // namespace mg
