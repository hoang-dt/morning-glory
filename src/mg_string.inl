#pragma once

#include <assert.h>
#include <string.h>
#include "mg_macros.h"

namespace mg {

mg_Inline stref::
stref() = default;

mg_Inline stref::
stref(cstr PtrIn, int SizeIn) 
  : ConstPtr(PtrIn), Size(SizeIn) {}

mg_Inline stref::
stref(cstr PtrIn) 
  : ConstPtr(PtrIn), Size(int(strlen(PtrIn))) {}

mg_Inline char& stref::
operator[](int Idx) { assert(Idx < Size); return Ptr[Idx]; }

mg_Inline stref::
operator bool() const { return Ptr != nullptr; }

mg_Inline str Begin   (stref Str) { return Str.Ptr; }
mg_Inline str End     (stref Str) { return Str.Ptr + Str.Size; }
mg_Inline str RevBegin(stref Str) { return Str.Ptr + Str.Size - 1; }
mg_Inline str RevEnd  (stref Str) { return Str.Ptr - 1; }

mg_Inline tokenizer::
tokenizer() = default;

mg_Inline tokenizer::
tokenizer(stref InputIn, stref DelimsIn)
  : Input(InputIn), Delims(DelimsIn), Pos(0) {}

mg_Inline void 
Init(tokenizer* Tk, stref Input, stref Delims) {
  Tk->Input = Input;
  Tk->Delims = Delims;
  Tk->Pos = 0;
}

} // namespace mg

