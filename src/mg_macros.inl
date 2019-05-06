#pragma once

namespace mg {
/* Count the number of times a character appears in a string. Return -1 for an
 * empty string. */
inline constexpr int
Count(const char* str, char c) {
  int count = 0;
  if (!(*str)) // empty string
    return -1;
  while (*str)
    count += (*str++ == c);
  return count;
}
} // namespace mg

#undef mg_Restrict
#if defined(__clang__) || defined(__GNUC__)
#define mg_Restrict __restrict__
#elif defined(_MSC_VER)
#define mg_Restrict __restrict
#endif

#undef mg_Inline
#if defined(_MSC_VER)
#define mg_Inline __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#define mg_Inline inline __attribute__((__always_inline__))
#endif
