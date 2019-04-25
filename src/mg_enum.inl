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
enum class enum_name : type { __VA_ARGS__, __Invalid__ };\
struct mg_Cat(enum_name, _s) {\
  enum_name Val;\
  struct enum_item {\
    str_ref Name;\
    enum_name ItemVal;\
  };\
  using name_map = array<enum_item, mg_NumArgs(__VA_ARGS__)>;\
  inline static name_map NameMap = []() {\
    name_map MyNameMap;\
    tokenizer Tk1(mg_Str(__VA_ARGS__), ",");\
    type CurrentVal = 0;\
    for (int I = 0; ; ++I, ++CurrentVal) {\
      str_ref Token = Next(&Tk1);\
      if (!Token) break;\
      tokenizer Tk2(Token, " =");\
      str_ref EnumStr = Next(&Tk2);\
      str_ref EnumVal = Next(&Tk2);\
      if (EnumVal) {\
        char* EndPtr = nullptr;\
        errno = 0;\
        enum_name MyVal = enum_name(strtol(EnumVal.Ptr, &EndPtr, 10));\
        if (errno == ERANGE || EndPtr == EnumVal.Ptr || !EndPtr ||\
            !(isspace(*EndPtr) || *EndPtr == ',' || *EndPtr == '\0'))\
          assert(false && " non-integer enum values");\
        else if (MyVal < static_cast<enum_name>(CurrentVal))\
          assert(false && " non-increasing enum values");\
        else\
          CurrentVal = static_cast<type>(MyVal);\
      }\
      assert(I < Size(MyNameMap));\
      MyNameMap[I] = enum_item{EnumStr, static_cast<enum_name>(CurrentVal)};\
    }\
    return MyNameMap;\
  }();\
  \
  mg_Cat(enum_name, _s)() : mg_Cat(enum_name, _s)(enum_name::__Invalid__) {}\
  mg_Cat(enum_name, _s)(enum_name Value) {\
    const auto* It = ConstBegin(NameMap);\
    while (It != ConstEnd(NameMap)) {\
      if (It->ItemVal == Value)\
        break;\
      ++It;\
    }\
    this->Val = (It != ConstEnd(NameMap)) ? It->ItemVal : enum_name::__Invalid__;\
  }\
  explicit mg_Cat(enum_name, _s)(str_ref Name) {\
    const auto* It = ConstBegin(NameMap);\
    while (It != ConstEnd(NameMap)) {\
      if (It->Name == Name)\
        break;\
      ++It;\
    }\
    Val = (It != ConstEnd(NameMap)) ? It->ItemVal : enum_name::__Invalid__;\
  }\
  explicit operator bool() const { return Val != enum_name::__Invalid__; }\
}; /* struct enum_name */\
\
inline str_ref ToString(enum_name Enum) {\
  mg_Cat(enum_name, _s) EnumS(Enum);\
  const auto* It = ConstBegin(EnumS.NameMap);\
  while (It != ConstEnd(EnumS.NameMap)) {\
    if (It->ItemVal == EnumS.Val)\
      break;\
    ++It;\
  }\
  assert(It != ConstEnd(EnumS.NameMap));\
  return It->Name;\
}\
\
template <>\
struct StringTo<enum_name> {\
  enum_name operator()(str_ref Name) {\
    mg_Cat(enum_name, _s) EnumS(Name);\
    return EnumS.Val;\
  }\
};\
\
} // namespace mg

