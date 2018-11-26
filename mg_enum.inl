#pragma once

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "mg_algorithm.h"
#include "mg_macros.h"
#include "mg_string.h"
#include "mg_types.h"

#undef mg_Enum
#define mg_Enum(enum_name, type, ...)\
namespace mg {\
\
struct enum_name {\
  enum : type { __VA_ARGS__, __Invalid__ };\
  type Value;\
  \
  struct enum_item {\
    string_ref Name;\
    type Value;\
  };\
  using name_map = array<enum_item, mg_NumArgs(__VA_ARGS__)>;\
  inline static name_map NameMap = []() {\
    name_map NameMap;\
    tokenizer Tk1(#__VA_ARGS__, ",");\
    type CurrentVal = 0;\
    for (int I = 0; ; ++I, ++CurrentVal) {\
      string_ref Token = Next(&Tk1);\
      if (!Token) break;\
      tokenizer Tk2(Token, " =");\
      string_ref EnumStr = Next(&Tk2);\
      string_ref EnumVal = Next(&Tk2);\
      if (EnumVal) {\
        char* EndPtr = nullptr;\
        errno = 0;\
        type Val = type(strtol(EnumVal.Ptr, &EndPtr, 10));\
        if (errno == ERANGE || (EndPtr == EnumVal.Ptr) || !Contains(string_ref(" ,\0", 3), *EndPtr))\
          assert(false && " non-integer enum values");\
        else if (Val < CurrentVal)\
          assert(false && " non-increasing enum values");\
        else\
          CurrentVal = Val;\
      }\
      assert(I < Size(NameMap));\
      NameMap[I] = enum_item{ EnumStr, CurrentVal };\
    }\
    return NameMap;\
  }();\
  \
  enum_name() : enum_name(__Invalid__) {}\
  enum_name(type Value) {\
    auto It = FindIf(Begin(NameMap), End(NameMap),\
      [Value](auto& Elem) { return Elem.Value == Value; });\
    if (It != End(NameMap)) \
      this->Value = It->Value;\
    else\
      this->Value = __Invalid__;\
  }\
  explicit enum_name(string_ref Name) {\
    auto It = FindIf(Begin(NameMap), End(NameMap),\
      [Name](auto& Elem) { return Elem.Name == Name; });\
    if (It != End(NameMap)) \
      Value = It->Value;\
    else\
      Value = __Invalid__;\
  }\
  explicit operator bool() { return Value != __Invalid__; }\
}; /* struct enum_name */\
\
inline string_ref ToString(enum_name Enum) {\
  auto It = FindIf(Begin(Enum.NameMap), End(Enum.NameMap),\
    [Enum](auto Elem) { return Elem.Value == Enum.Value; });\
  assert(It != End(Enum.NameMap));\
  return It->Name;\
}\
inline bool operator==(enum_name Lhs, enum_name Rhs) { return Lhs.Value == Rhs.Value; }\
inline bool operator!=(enum_name Lhs, enum_name Rhs) { return Lhs.Value != Rhs.Value; }\
inline bool operator< (enum_name Lhs, enum_name Rhs) { return Lhs.Value <  Rhs.Value; }\
\
} // namespace mg
