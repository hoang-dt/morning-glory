#pragma once

#define mg_ArraySize(x) sizeof(x) / sizeof(*(x))

namespace mg {

/* Count the number of times a character appears in a string */
constexpr int CountOccurrences(const char* str, char c) {
  int count = 0;
  while (*str)
    count += (*str++ == c);
  return count;
}

/* Return the number of comma-separated arguments */
#define mg_NumArgs(...) (CountOccurrences(#__VA_ARGS__, ',') + 1)

}
