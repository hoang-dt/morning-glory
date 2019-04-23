#pragma once

#include "mg_types.h"

#define MaxSlots 16

namespace mg {

struct logger {
  array<FILE*, MaxSlots> FileHandles = {};
  array<cstr, MaxSlots> FileNames = {};
  array<u32, MaxSlots> FileNameHashes = {};
  buffer_mode Mode = buffer_mode::Full;
};

constexpr bool IsStdErr(cstr Input) {
  return Input[0] == 's' && Input[1] == 't' && Input[2] == 'd' &&
         Input[3] == 'e' && Input[4] == 'r' && Input[5] == 'r';
}
constexpr bool IsStdOut(cstr Input) {
  return Input[0] == 's' && Input[1] == 't' && Input[2] == 'd' &&
         Input[3] == 'o' && Input[4] == 'u' && Input[5] == 't';
}

template <typename t>
constexpr cstr CastCStr(t Input) {
  if constexpr (IsSameType<t, cstr>::Result)
    return Input;
  return nullptr;
}

} // namespace mg

#undef mg_Log
#if defined(mg_Verbose)
#define mg_Log(FileName, Format, ...) {\
  FILE* Fp = nullptr;\
  if constexpr (IsStdErr(#FileName))\
    Fp = stderr;\
  else if constexpr (IsStdOut(#FileName))\
    Fp = stdout;\
  else\
    Fp = mg::GetFileHandle(&mg::GlobalLogger(), CastCStr(FileName));\
  mg::printer Pr(Fp);\
  mg_Print(&Pr, Format, __VA_ARGS__);\
}
#else
#define mg_Log(FileName, Format, ...)
#endif

#undef MaxSlots
