#pragma once

#undef mg_ArraySize
#define mg_ArraySize(x) sizeof(x) / sizeof(*(x))

namespace mg {
/* Count the number of times a character appears in a string. Return -1 for an empty string. */
constexpr int mg_CountOccurrences(const char* str, char c) {
  int count = 0;
  if (!(*str)) // empty string
    return -1;
  while (*str)
    count += (*str++ == c);
  return count;
}
}

#undef mg_NumArgs
#define mg_NumArgs(...) (mg_CountOccurrences(#__VA_ARGS__, ',') + 1)
