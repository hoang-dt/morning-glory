/* String processing utilities */

#pragma once

#include "mg_types.h"

namespace mg {

/* Useful to create a string_ref out of a literal string */
#define mg_StringRef(x) mg::string_ref((x), sizeof(x) - 1)

/* A "view" into a (usually bigger) null-terminated string. A string_ref itself is not null-terminated.
There are two preferred ways to construct a string_ref from a char[] array:
  - Use the mg_StringRef macro to make string_ref refer to the entire array
  - Use the string_ref(const char*) constructor to only refer up to the first NULL character */
struct string_ref {
  union {
    char* Ptr = nullptr;
    const char* PtrC;
  };
  int Size = 0;

  string_ref() = default;
  string_ref(cstr Ptr, int Size);
  string_ref(cstr Ptr);
  char& operator[](int i);
  operator bool();
}; // struct string_ref

char* Begin(string_ref Str);
char* End(string_ref Str);

bool operator==(string_ref Lhs, string_ref Rhs);

/* Return a substring of a given string. The substring starts at Begin and has length Size.
Return the empty string if no proper substring can be constructed (e.g. Begin >= Str.Size). */
string_ref SubString(string_ref Str, int Begin, int Size);

/* Tokenize strings without allocating memory */
struct tokenizer {
  string_ref Input;
  string_ref Delims;
  int Pos = 0;

  tokenizer() = default;
  tokenizer(string_ref Input, string_ref Delims = " \n\t");
}; // struct tokenizer

void Init(tokenizer* Tk, string_ref Input, string_ref Delims = " \n\t");
string_ref Next(tokenizer* tk);
void Reset(tokenizer* tk);

}
