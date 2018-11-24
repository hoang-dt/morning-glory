/* An enum type that knows how to convert its elements from and to strings */

#pragma once

#define mg_Enum(enum_name, type, ...)\
namespace mg {\
\
struct enum_name {\
  enum : type {__VA_ARGS__};\
  type Value;\
  \
  enum_name();\
  enum_name(type Value);\
  enum_name& operator=(type Value);\
  explicit enum_name(string_ref Name);\
}; /* struct enum_name */\
\
string_ref ToString(enum_name Enum);\
bool operator==(enum_name Lhs, enum_name Rhs);\
bool operator!=(enum_name Lhs, enum_name Rhs);\
bool operator< (enum_name Lhs, enum_name Rhs);\
\
} // namespace mg

#include "mg_enum.inl"
