#pragma once

namespace mg {
/* Count the number of times a character appears in a string. Return -1 for an empty string. */
constexpr int CountOccurrences(const char* str, char c) {
  int count = 0;
  if (!(*str)) // empty string
    return -1;
  while (*str)
    count += (*str++ == c);
  return count;
}
} // namespace mg
