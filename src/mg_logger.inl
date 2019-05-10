#pragma once

namespace mg {

constexpr inline bool
IsStdErr(cstr Input) {
  return Input[0] == 's' && Input[1] == 't' && Input[2] == 'd' &&
         Input[3] == 'e' && Input[4] == 'r' && Input[5] == 'r';
}
constexpr inline bool
IsStdOut(cstr Input) {
  return Input[0] == 's' && Input[1] == 't' && Input[2] == 'd' &&
         Input[3] == 'o' && Input[4] == 'u' && Input[5] == 't';
}

mg_T(t) constexpr cstr
CastCStr(t Input) {
  if constexpr (is_same_type<t, cstr>::Value)
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

#undef mg_MaxSlots
