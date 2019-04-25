#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_string.h"

namespace mg {

str_ref::str_ref() = default;
str_ref::str_ref(cstr PtrIn, int SizeIn) : ConstPtr(PtrIn), Size(SizeIn) {}
str_ref::str_ref(cstr PtrIn) : ConstPtr(PtrIn), Size(int(strlen(PtrIn))) {}
char& str_ref::operator[](int Idx) { mg_Assert(Idx < Size); return Ptr[Idx]; }
char str_ref::operator[](int Idx) const { mg_Assert(Idx < Size); return Ptr[Idx]; }
str_ref::operator bool() const { return Ptr != nullptr; }

str ToString(str_ref Str) {
  mg_Assert(Str.Size < (int)sizeof(ScratchBuf));
  if (Str.Ptr != ScratchBuf)
    snprintf(ScratchBuf, sizeof(ScratchBuf), "%.*s", Str.Size, Str.Ptr);
  return ScratchBuf;
}
str Begin(str_ref Str) { return Str.Ptr; }
cstr ConstBegin(str_ref Str) { return Str.ConstPtr; }
str End(str_ref Str) { return Str.Ptr + Str.Size; }
cstr ConstEnd(str_ref Str) { return Str.ConstPtr + Str.Size; }
str ReverseBegin(str_ref Str) { return Str.Ptr + Str.Size - 1; }
cstr ConstReverseBegin(str_ref Str) { return Str.ConstPtr + Str.Size - 1; }
str ReverseEnd(str_ref Str) { return Str.Ptr - 1; }
cstr ConstReverseEnd(str_ref Str) { return Str.ConstPtr - 1; }

bool operator==(str_ref Lhs, str_ref Rhs) {
  if (Lhs.Size != Rhs.Size)
    return false;
  for (int I = 0; I < Lhs.Size; ++I) {
    if (Lhs[I] != Rhs[I])
      return false;
  }
  return true;
}

str_ref TrimLeft(str_ref Str) {
  str_ref StrOut = Str;
  while (StrOut.Size && isspace(*StrOut.Ptr)) {
    ++StrOut.Ptr;
    --StrOut.Size;
  }
  return StrOut;
}

str_ref TrimRight(str_ref Str) {
  str_ref StrOut = Str;
  while (StrOut.Size && isspace(StrOut[StrOut.Size - 1]))
    --StrOut.Size;
  return StrOut;
}

str_ref Trim(str_ref Str) {
  return TrimLeft(TrimRight(Str));
}

str_ref SubString(str_ref Str, int Begin, int Size) {
  if (!Str || Begin >= Str.Size)
    return str_ref();
  return str_ref(Str.Ptr + Begin, Min(Size, Str.Size));
}

void Copy(str_ref Dst, str_ref Src, bool AddNull) {
  int NumBytes = Min(Dst.Size, Src.Size);
  memcpy(Dst.Ptr, Src.Ptr, size_t(NumBytes));
  if (AddNull)
    Dst.Ptr[NumBytes] = 0;
}

bool ToInt(str_ref Str, int* Result) {
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

bool ToDouble(str_ref Str, f64* Result) {
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
tokenizer::tokenizer(str_ref InputIn, str_ref DelimsIn)
  : Input(InputIn), Delims(DelimsIn), Pos(0) {}

void Init(tokenizer* Tk, str_ref Input, str_ref Delims) {
  Tk->Input = Input;
  Tk->Delims = Delims;
  Tk->Pos = 0;
}

str_ref Next(tokenizer* Tk) {
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
  return str_ref();
}

void Reset(tokenizer* Tk) {
  Tk->Pos = 0;
}

}
