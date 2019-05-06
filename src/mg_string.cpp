#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_string.h"

namespace mg {

stref::stref() = default;
stref::stref(cstr PtrIn, int SizeIn) : ConstPtr(PtrIn), Size(SizeIn) {}
stref::stref(cstr PtrIn) : ConstPtr(PtrIn), Size(int(strlen(PtrIn))) {}
char& stref::operator[](int Idx) { mg_Assert(Idx < Size); return Ptr[Idx]; }
stref::operator bool() { return Ptr != nullptr; }

str ToString(stref Str) {
  mg_Assert(Str.Size < (int)sizeof(ScratchBuf));
  if (Str.Ptr != ScratchBuf)
    snprintf(ScratchBuf, sizeof(ScratchBuf), "%.*s", Str.Size, Str.Ptr);
  return ScratchBuf;
}
str Begin(stref Str) { return Str.Ptr; }
str End(stref Str) { return Str.Ptr + Str.Size; }
str RevBegin(stref Str) { return Str.Ptr + Str.Size - 1; }
str RevEnd(stref Str) { return Str.Ptr - 1; }

bool operator==(stref Lhs, stref Rhs) {
  if (Lhs.Size != Rhs.Size)
    return false;
  for (int I = 0; I < Lhs.Size; ++I) {
    if (Lhs[I] != Rhs[I])
      return false;
  }
  return true;
}

stref TrimLeft(stref Str) {
  stref StrOut = Str;
  while (StrOut.Size && isspace(*StrOut.Ptr)) {
    ++StrOut.Ptr;
    --StrOut.Size;
  }
  return StrOut;
}

stref TrimRight(stref Str) {
  stref StrOut = Str;
  while (StrOut.Size && isspace(StrOut[StrOut.Size - 1]))
    --StrOut.Size;
  return StrOut;
}

stref Trim(stref Str) {
  return TrimLeft(TrimRight(Str));
}

stref SubString(stref Str, int Begin, int Size) {
  if (!Str || Begin >= Str.Size)
    return stref();
  return stref(Str.Ptr + Begin, Min(Size, Str.Size));
}

void Copy(stref Dst, stref Src, bool AddNull) {
  int NumBytes = Min(Dst.Size, Src.Size);
  memcpy(Dst.Ptr, Src.Ptr, size_t(NumBytes));
  if (AddNull)
    Dst.Ptr[NumBytes] = 0;
}

bool ToInt(stref Str, int* Result) {
  if (!Str || Str.Size <= 0)
    return false;

  int Mult = 1, Start = 0;
  if (Str[0] == '-') {
    Mult = -1;
    Start = 1;
  }
  *Result = 0;
  for (int I = Start; I < Str.Size; ++I) {
    int V = Str[Str.Size - I - 1] - '0';
    if (V >= 0 && V < 10)
      *Result += Mult * (V * Pow10[I]);
    else
      return false;
  }
  return true;
}

bool ToDouble(stref Str, f64* Result) {
  if (!Str || Str.Size <= 0)
    return false;
  char* EndPtr = nullptr;
  *Result = strtod(Str.ConstPtr, &EndPtr);
  bool Failure = errno == ERANGE || EndPtr == Str.ConstPtr || !EndPtr ||
                 !(isspace(*EndPtr) || ispunct(*EndPtr) || (*EndPtr) == 0);
  return !Failure;
}

/* tokenizer stuff */

tokenizer::tokenizer() = default;
tokenizer::tokenizer(stref InputIn, stref DelimsIn)
  : Input(InputIn), Delims(DelimsIn), Pos(0) {}

void Init(tokenizer* Tk, stref Input, stref Delims) {
  Tk->Input = Input;
  Tk->Delims = Delims;
  Tk->Pos = 0;
}

stref Next(tokenizer* Tk) {
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

void Reset(tokenizer* Tk) {
  Tk->Pos = 0;
}

}
