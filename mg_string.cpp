#include <ctype.h>
#include <string.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_string.h"

namespace mg {

string_ref::string_ref(cstr Ptr, int Size) : ConstPtr(Ptr), Size(Size) {}
string_ref::string_ref(cstr Ptr) : ConstPtr(Ptr), Size(strlen(Ptr)) {}
char& string_ref::operator[](int Idx) { mg_Assert(Idx < Size); return Ptr[Idx]; }
char string_ref::operator[](int Idx) const { mg_Assert(Idx < Size); return Ptr[Idx]; }
string_ref::operator bool() const { return Ptr != nullptr; }

str ToString(string_ref Str) {
  mg_Assert(Str.Size < (int)sizeof(ScratchBuffer));
  if (Str.Ptr != ScratchBuffer)
    snprintf(ScratchBuffer, sizeof(ScratchBuffer), "%.*s", Str.Size, Str.Ptr);
  return ScratchBuffer;
}
str Begin(string_ref Str) { return Str.Ptr; }
cstr ConstBegin(string_ref Str) { return Str.ConstPtr; }
str End(string_ref Str) { return Str.Ptr + Str.Size; }
cstr ConstEnd(string_ref Str) { return Str.ConstPtr + Str.Size; }
str ReverseBegin(string_ref Str) { return Str.Ptr + Str.Size - 1; }
cstr ConstReverseBegin(string_ref Str) { return Str.ConstPtr + Str.Size - 1; }
str ReverseEnd(string_ref Str) { return Str.Ptr - 1; }
cstr ConstReverseEnd(string_ref Str) { return Str.ConstPtr - 1; }

bool operator==(string_ref Lhs, string_ref Rhs) {
  if (Lhs.Size != Rhs.Size)
    return false;
  for (int I = 0; I < Lhs.Size; ++I) {
    if (Lhs[I] != Rhs[I])
      return false;
  }
  return true;
}

string_ref TrimLeft(string_ref Str) {
  string_ref StrOut = Str;
  while (StrOut.Size && isspace(*StrOut.Ptr)) {
    ++StrOut.Ptr;
    --StrOut.Size;
  }
  return StrOut;
}

string_ref TrimRight(string_ref Str) {
  string_ref StrOut = Str;
  while (StrOut.Size && isspace(StrOut[StrOut.Size - 1]))
    --StrOut.Size;
  return StrOut;
}

string_ref Trim(string_ref Str) {
  return TrimLeft(TrimRight(Str));
}

string_ref SubString(string_ref Str, int Begin, int Size) {
  if (!Str || Begin >= Str.Size)
    return string_ref();
  return string_ref(Str.Ptr + Begin, Min(Size, Str.Size));
}

void Copy(string_ref Dst, string_ref Src, bool AddNull) {
  int NumBytes = Min(Dst.Size, Src.Size);
  memcpy(Dst.Ptr, Src.Ptr, NumBytes);
  if (AddNull)
    Dst.Ptr[NumBytes] = 0;
}

bool ToInt(string_ref Str, int* Result) {
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

/* tokenizer stuff */

tokenizer::tokenizer(string_ref Input, string_ref Delims)
  : Input(Input), Delims(Delims), Pos(0) {}

void Init(tokenizer* Tk, string_ref Input, string_ref Delims) {
  Tk->Input = Input;
  Tk->Delims = Delims;
  Tk->Pos = 0;
}

string_ref Next(tokenizer* Tk) {
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
  return string_ref();
}

void Reset(tokenizer* Tk) {
  Tk->Pos = 0;
}

}
