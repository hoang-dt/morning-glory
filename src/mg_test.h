#pragma once

#include <functional>
#include <map>
#include <stdio.h>
#include "mg_common.h"

namespace mg {

using TestFunc = std::function<void(void)>;
inline std::map<cstr, TestFunc> TestFuncMap;

#define mg_RegisterTest(Name, Func)\
inline bool __Test##Name__ = []() {\
  mg::TestFuncMap[Name] = Func;\
  return true;\
}();

#define mg_TestMain \
int main() {\
  for (const auto& Test : mg::TestFuncMap) {\
    printf("Testing %s:\n", Test.first);\
    Test.second();\
  }\
  return 0;\
}

} // namespace mg
