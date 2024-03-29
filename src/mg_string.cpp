#include <ctype.h>
#include <stdlib.h>
#include "mg_algorithm.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_string.h"

namespace mg {

cstr
ToString(const stref& Str) {
  mg_Assert(Str.Size < (int)sizeof(ScratchBuf));
  if (Str.Ptr != ScratchBuf)
    snprintf(ScratchBuf, sizeof(ScratchBuf), "%.*s", Str.Size, Str.Ptr);
  return ScratchBuf;
}

bool
operator==(const stref& Lhs, const stref& Rhs) {
  stref& LhsR = const_cast<stref&>(Lhs);
  stref& RhsR = const_cast<stref&>(Rhs);
  if (LhsR.Size != RhsR.Size)
    return false;
  for (int I = 0; I < LhsR.Size; ++I) {
    if (LhsR[I] != RhsR[I])
      return false;
  }
  return true;
}

stref
TrimLeft(const stref& Str) {
  stref StrOut = Str;
  while (StrOut.Size && isspace(*StrOut.Ptr)) {
    ++StrOut.Ptr;
    --StrOut.Size;
  }
  return StrOut;
}

stref 
TrimRight(const stref& Str) {
  stref StrOut = Str;
  while (StrOut.Size && isspace(StrOut[StrOut.Size - 1]))
    --StrOut.Size;
  return StrOut;
}

stref 
Trim(const stref& Str) { return TrimLeft(TrimRight(Str)); }

stref 
SubString(const stref& Str, int Begin, int Size) {
  if (!Str || Begin >= Str.Size)
    return stref();
  return stref(Str.Ptr + Begin, Min(Size, Str.Size));
}

void 
Copy(const stref& Src, stref* Dst, bool AddNull) {
  int NumBytes = Min(Dst->Size, Src.Size);
  memcpy(Dst->Ptr, Src.Ptr, size_t(NumBytes));
  if (AddNull)
    Dst->Ptr[NumBytes] = 0;
}

bool 
ToInt(const stref& Str, int* Result) {
  stref& StrR = const_cast<stref&>(Str);
  if (!StrR || StrR.Size <= 0)
    return false;

  int Mult = 1, Start = 0;
  if (StrR[0] == '-') {
    Mult = -1;
    Start = 1;
  }
  *Result = 0;
  for (int I = Start; I < Str.Size; ++I) {
    int V = StrR[StrR.Size - I - 1] - '0';
    if (V >= 0 && V < 10)
      *Result += Mult * (V * pow<int, 10>()[I]);
    else
      return false;
  }
  return true;
}

bool 
ToDouble(const stref& Str, f64* Result) {
  if (!Str || Str.Size <= 0)
    return false;
  char* EndPtr = nullptr;
  *Result = strtod(Str.ConstPtr, &EndPtr);
  bool Failure = errno == ERANGE || EndPtr == Str.ConstPtr || !EndPtr ||
                 !(isspace(*EndPtr) || ispunct(*EndPtr) || (*EndPtr) == 0);
  return !Failure;
}

/* tokenizer stuff */

stref 
Next(tokenizer* Tk) {
  while (Tk->Pos < Tk->Input.Size && Contains(Tk->Delims, Tk->Input[Tk->Pos]))
    ++Tk->Pos;

  if (Tk->Pos < Tk->Input.Size) {
    int Length = 0;
    while (Tk->Pos < Tk->Input.Size && !Contains(Tk->Delims, Tk->Input[Tk->Pos])) {
      ++Tk->Pos;
      ++Length;
    }
    return SubString(Tk->Input, Tk->Pos - Length, Length);
  }
  return stref();
}

void 
Reset(tokenizer* Tk) { Tk->Pos = 0; }

}
