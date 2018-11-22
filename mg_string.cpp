#include <string.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_string.h"

namespace mg {

string_ref::string_ref(cstr Ptr, int Size) : PtrC(Ptr), Size(Size) {}
string_ref::string_ref(cstr Ptr) : PtrC(Ptr), Size(strlen(Ptr)) {}
char& string_ref::operator[](int Idx) { mg_Assert(Idx < Size); return Ptr[Idx]; }
string_ref::operator bool() { return Ptr != nullptr; }

char* Begin(string_ref Str) { return Str.Ptr; }
char* End(string_ref Str) { return Str.Ptr + Str.Size; }

bool operator==(string_ref Lhs, string_ref Rhs) {
  if (!Lhs || !Rhs || Lhs.Size != Rhs.Size)
    return false;
  for (int I = 0; I < Lhs.Size; ++I) {
    if (Lhs[I] != Rhs[I])
      return false;
  }
  return true;
}

string_ref SubString(string_ref Str, int Begin, int Size) {
  if (!Str || Begin >= Str.Size)
    return string_ref();
  return string_ref(Str.Ptr + Begin, Min(Size, Str.Size));
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
