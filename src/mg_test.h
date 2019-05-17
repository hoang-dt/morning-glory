#pragma once

#include <functional>
#include <map>
#include <stdio.h>
#include "mg_common.h"

namespace mg {

// TODO: just  use regular function pointers
// TODO: use array instead of map
using TestFunc = std::function<void(void)>;
inline std::map<cstr, TestFunc> TestFuncMap;

#define mg_RegisterTest(Name, Func)\
bool Test##Name() {\
  mg::TestFuncMap[#Name] = Func;\
  return true;\
}\
inline bool VarTest##Name = Test##Name();

#define mg_RegisterTestOnly(Name, Func)\
bool Test##Name() {\
  mg::TestFuncMap.clear();\
  mg::TestFuncMap[#Name] = Func;\
  return true;\
}\
inline bool VarTest##Name = Test##Name();

#define mg_TestMain \
int main() {\
  for (const auto& Test : mg::TestFuncMap) {\
    printf("-------- Testing %s:\n", Test.first);\
    Test.second();\
    printf("PASSED\n");\
  }\
  return 0;\
}

} // namespace mg

