/*
An enum type that knows how to convert from and to strings. There is always a
special __Invalid__ enum item at the end.
NOTE: No checking is done for duplicate values. */

#pragma once

#include "mg_macros.h"
#include "mg_string.h"

#define mg_Enum(enum_name, type, ...)\
namespace mg {\
struct enum_name {\
  enum : type { __Invalid__, __VA_ARGS__ };\
  type Val;\
  enum_name();\
  enum_name(type Val);\
  enum_name& operator=(type Val);\
  explicit enum_name(stref Name);\
  explicit operator bool() const;\
}; /* struct enum_name */\
\
stref ToString(enum_name Enum);\
} // namespace mg

namespace mg {
/* Construct an enum from a string */
mg_T(t) struct StringTo { t operator()(stref Name); };
}

#include "mg_enum.inl"
