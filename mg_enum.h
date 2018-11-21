#pragma once

#include <errno.h>
#include <stdlib.h>
#include "mg_algorithm.h"
#include "mg_assert.h"
#include "mg_macros.h"
#include "mg_string.h"
#include "mg_types.h"

#define mg_Enum(enum_name, type, ...)\
namespace mg {\
\
struct enum_item {\
  string_ref Name;\
  type Value;\
};\
\
using name_map = array<enum_item, mg_NumArgs(__VA_ARGS__)>;\
auto GenerateNameMap = []() {\
  name_map NameMap;\
  tokenizer Tk1(#__VA_ARGS__, ",");\
  type CurrentVal = 0;\
  for (int I = 0; ; ++I, ++CurrentVal) {\
    string_ref Token = Next(&Tk1);\
    if (!Token) break;\
    tokenizer Tk2(Token, " =");\
    string_ref EnumStr = Next(&Tk2);\
    string_ref EnumVal = Next(&Tk2);\
    if (!EnumVal) {\
      char* EndPtr = nullptr;\
      errno = 0;\
      type Val = type(strtol(EnumVal.Ptr, &EndPtr, 10));\
      if (errno == ERANGE || (EndPtr == EnumVal.Ptr) || !Contains(string_ref(" ,\0", 3), *EndPtr))\
        mg_Assert(false);\
      else\
        CurrentVal = Val;\
    }\
    mg_Assert(I < Size(NameMap));\
    NameMap[I] = enum_item{ EnumStr, CurrentVal };\
  }\
  return NameMap;\
};\
\
struct enum_name {\
  enum : type {__VA_ARGS__};\
  type Value;\
  inline static name_map NameMap = GenerateNameMap();\
  \
  enum_name() : enum_name(NameMap[0].Value) {}\
  enum_name(type Value) : Value(Value) {}\
  enum_name& operator=(type Value) { this->Value = Value; return *this; }\
}; /* struct enum_name */\
\
enum_name FromString(string_ref Name) {\
  auto It = FindIf(Begin(enum_name::NameMap), End(enum_name::NameMap),\
    [Name](auto Elem) { return Elem.Name == Name; });\
  mg_Assert(It != End(enum_name::NameMap));\
  return enum_name(It->Value);\
}\
string_ref ToString(enum_name Enum) {\
  auto It = FindIf(Begin(Enum.NameMap), End(Enum.NameMap),\
    [Enum](auto Elem) { return Elem.Value == Enum.Value; });\
  mg_Assert(It != End(Enum.NameMap));\
  return It->Name;\
}\
inline bool operator==(enum_name Lhs, enum_name Rhs) { return Lhs.Value == Rhs.Value; }\
inline bool operator!=(enum_name Lhs, enum_name Rhs) { return Lhs.Value != Rhs.Value; }\
inline bool operator< (enum_name Lhs, enum_name Rhs) { return Lhs.Value <  Rhs.Value; }\
\
}
