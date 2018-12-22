/* String processing utilities */

#pragma once

#include "mg_types.h"

namespace mg {

/* Useful to create a string_ref out of a literal string or a fixed-size character array */
#define mg_StringRef(x) mg::string_ref((x), sizeof(x) - 1)

/* A "view" into a (usually bigger) null-terminated string. A string_ref itself is not null-terminated.
There are two preferred ways to construct a string_ref from a char[] array:
  - Use the mg_StringRef macro to make string_ref refer to the entire array
  - Use the string_ref(const char*) constructor to only refer up to the first NULL character */
struct string_ref {
  union {
    str Ptr = nullptr;
    cstr ConstPtr ;
  };
  int Size = 0;

  string_ref();
  string_ref(cstr Ptr, int Size);
  string_ref(cstr Ptr);
  char& operator[](int Idx);
  char operator[](int Idx) const;
  operator bool() const;
}; // struct string_ref

str ToString(string_ref Str);
str Begin(string_ref Str);
cstr ConstBegin(string_ref Str);
str End(string_ref Str);
cstr ConstEnd(string_ref Str);
str ReverseBegin(string_ref Str);
cstr ConstReverseBegin(string_ref Str);
str ReverseEnd(string_ref Str);
const char* ConstReverseEnd(string_ref Str);
bool operator==(string_ref Lhs, string_ref Rhs);

/* Remove spaces at the start of a string */
string_ref TrimLeft(string_ref Str);
string_ref TrimRight(string_ref Str);
string_ref Trim(string_ref Str);
/* Return a substring of a given string. The substring starts at Begin and has length Size.
Return the empty string if no proper substring can be constructed (e.g. Begin >= Str.Size). */
string_ref SubString(string_ref Str, int Begin, int Size);
/* Copy the underlying buffer referred to by Src to the one referred to by Dst. AddNull
should be true whenever dst represents a whole string (as opposed to a substring). If Src is
larger than Dst, we copy as many characters as we can. We always assume that the null
character can be optionally added without overflowing the memory of Dst. */
void Copy(string_ref Dst, string_ref Src, bool AddNull = true);
/* Parse a string_ref and return a number */
bool ToInt(string_ref Str, int* Result);
bool ToDouble(string_ref Str, f64* Result);

/* Tokenize strings without allocating memory */
struct tokenizer {
  string_ref Input;
  string_ref Delims;
  int Pos = 0;

  tokenizer();
  tokenizer(string_ref Input, string_ref Delims = " \n\t");
}; // struct tokenizer

void Init(tokenizer* Tk, string_ref Input, string_ref Delims = " \n\t");
string_ref Next(tokenizer* tk);
void Reset(tokenizer* tk);

} // namespace mg
