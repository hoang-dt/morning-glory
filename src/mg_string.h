/* String processing utilities */

#pragma once

#include "mg_types.h"

namespace mg {

/* Useful to create a string_ref out of a literal string */
#define mg_StringRef(x) mg::str_ref((x), sizeof(x) - 1)

/* A "view" into a (usually bigger) null-terminated string. A string_ref itself
 * is not null-terminated.
 * There are two preferred ways to construct a string_ref from a char[] array:
 *  - Use the mg_StringRef macro to make string_ref refer to the entire array
 *  - Use the string_ref(const char*) constructor to refer up to the first NULL */
struct str_ref {
  union {
    str Ptr = nullptr;
    cstr ConstPtr ;
  };
  int Size = 0;

  str_ref();
  str_ref(cstr Ptr, int Size);
  str_ref(cstr Ptr);
  char& operator[](int Idx);
  char operator[](int Idx) const;
  operator bool() const;
}; // struct string_ref

str ToString(str_ref Str);
str Begin(str_ref Str);
cstr ConstBegin(str_ref Str);
str End(str_ref Str);
cstr ConstEnd(str_ref Str);
str ReverseBegin(str_ref Str);
cstr ConstReverseBegin(str_ref Str);
str ReverseEnd(str_ref Str);
const char* ConstReverseEnd(str_ref Str);
bool operator==(str_ref Lhs, str_ref Rhs);

/* Remove spaces at the start of a string */
str_ref TrimLeft(str_ref Str);
str_ref TrimRight(str_ref Str);
str_ref Trim(str_ref Str);
/* Return a substring of a given string. The substring starts at Begin and has
 * length Size. Return the empty string if no proper substring can be constructed
 * (e.g. Begin >= Str.Size).
 */
str_ref SubString(str_ref Str, int Begin, int Size);
/* Copy the underlying buffer referred to by Src to the one referred to by Dst.
 * AddNull should be true whenever dst represents a whole string (as opposed to
 * a substring). If Src is larger than Dst, we copy as many characters as we can.
 * We always assume that the null character can be optionally added without
 * overflowing the memory of Dst.
 */
void Copy(str_ref Dst, str_ref Src, bool AddNull = true);
/* Parse a string_ref and return a number */
bool ToInt(str_ref Str, int* Result);
bool ToDouble(str_ref Str, f64* Result);

/* Tokenize strings without allocating memory */
struct tokenizer {
  str_ref Input;
  str_ref Delims;
  int Pos = 0;

  tokenizer();
  tokenizer(str_ref Input, str_ref Delims = " \n\t");
}; // struct tokenizer

void Init(tokenizer* Tk, str_ref Input, str_ref Delims = " \n\t");
str_ref Next(tokenizer* Tk);
void Reset(tokenizer* Tk);

} // namespace mg
