#pragma once

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
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
        if (errno == ERANGE || EndPtr == EnumVal.Ptr || !EndPtr ||\
          !(isspace(*EndPtr) || *EndPtr == ',' || *EndPtr == '\0'))\
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
    const auto* It = ConstBegin(NameMap);\
    while (It != ConstEnd(NameMap)) {\
      if (It->Value == Value)\
        break;\
      ++It;\
    }\
    if (It != ConstEnd(NameMap)) \
      this->Value = It->Value;\
    else\
      this->Value = __Invalid__;\
  }\
  explicit enum_name(string_ref Name) {\
    const auto* It = ConstBegin(NameMap);\
    while (It != ConstEnd(NameMap)) {\
      if (It->Name == Name)\
        break;\
      ++It;\
    }\
    if (It != ConstEnd(NameMap)) \
      Value = It->Value;\
    else\
      Value = __Invalid__;\
  }\
  explicit operator bool() const { return Value != __Invalid__; }\
}; /* struct enum_name */\
\
inline string_ref ToString(enum_name Enum) {\
  const auto* It = ConstBegin(Enum.NameMap);\
  while (It != ConstEnd(Enum.NameMap)) {\
    if (It->Value == Enum.Value)\
      break;\
    ++It;\
  }\
  assert(It != ConstEnd(Enum.NameMap));\
  return It->Name;\
}\
inline bool operator==(enum_name Lhs, enum_name Rhs) { return Lhs.Value == Rhs.Value; }\
inline bool operator!=(enum_name Lhs, enum_name Rhs) { return Lhs.Value != Rhs.Value; }\
inline bool operator< (enum_name Lhs, enum_name Rhs) { return Lhs.Value <  Rhs.Value; }\
\
} // namespace mg
