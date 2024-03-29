#pragma once

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "mg_common.h"

#undef mg_Enum
#define mg_Enum(enum_name, type, ...)\
namespace mg {\
\
enum class enum_name : type { __VA_ARGS__, __Invalid__ };\
\
struct mg_Cat(enum_name, _s) {\
  enum_name Val;\
  struct enum_item {\
    stref Name;\
    enum_name ItemVal;\
  };\
  \
  using name_map = stack_array<enum_item, mg_NumArgs(__VA_ARGS__)>;\
  \
  inline static name_map NameMap = []() {\
    name_map MyNameMap;\
    tokenizer Tk1(mg_Str(__VA_ARGS__), ",");\
    type CurrentVal = 0;\
    for (int I = 0; ; ++I, ++CurrentVal) {\
      stref Token = Next(&Tk1);\
      if (!Token) break;\
      tokenizer Tk2(Token, " =");\
      stref EnumStr = Next(&Tk2);\
      stref EnumVal = Next(&Tk2);\
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
  \
  mg_Cat(enum_name, _s)(enum_name Value) {\
    auto* It = Begin(NameMap);\
    while (It != End(NameMap)) {\
      if (It->ItemVal == Value)\
        break;\
      ++It;\
    }\
    this->Val = (It != End(NameMap)) ? It->ItemVal : enum_name::__Invalid__;\
  }\
  \
  explicit mg_Cat(enum_name, _s)(stref Name) {\
    auto* It = Begin(NameMap);\
    while (It != End(NameMap)) {\
      if (It->Name == Name)\
        break;\
      ++It;\
    }\
    Val = (It != End(NameMap)) ? It->ItemVal : enum_name::__Invalid__;\
  }\
  \
  explicit operator bool() const { return Val != enum_name::__Invalid__; }\
};\
\
inline stref \
ToString(enum_name Enum) {\
  mg_Cat(enum_name, _s) EnumS(Enum);\
  auto* It = Begin(EnumS.NameMap);\
  while (It != End(EnumS.NameMap)) {\
    if (It->ItemVal == EnumS.Val)\
      break;\
    ++It;\
  }\
  assert(It != End(EnumS.NameMap));\
  return It->Name;\
}\
\
template <>\
struct StringTo<enum_name> {\
  enum_name operator()(stref Name) {\
    mg_Cat(enum_name, _s) EnumS(Name);\
    return EnumS.Val;\
  }\
};\
\
} // namespace mg

