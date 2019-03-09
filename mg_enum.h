/* An enum type that knows how to convert its items from and to strings. 
 * There is always a special __Invalid__ enum item at the end. No checking is 
 * done for duplicate values. */

#pragma once

#include "mg_string.h"

#define mg_Enum(enum_name, type, ...)\
namespace mg {\
struct enum_name {\
  enum : type { __Invalid__, __VA_ARGS__ };\
  type Val;\
  enum_name();\
  enum_name(type Val);\
  enum_name& operator=(type Val);\
  explicit enum_name(string_ref Name);\
  explicit operator bool() const;\
}; /* struct enum_name */\
\
string_ref ToString(enum_name Enum);\
} // namespace mg

namespace mg {
template <typename t>
struct StringTo { t operator()(string_ref Name); };
}


#include "mg_enum.inl"
