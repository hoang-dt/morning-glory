#pragma once

#include "mg_macros.h"

namespace mg {

mg_T(func_t)
struct scope_guard {
  func_t Func;
  bool Dismissed = false;
  // TODO: std::forward FuncIn?
  scope_guard(const func_t& FuncIn) : Func(FuncIn) {}
  ~scope_guard() { if (!Dismissed) { Func(); } }
};

} // namespace mg

#define mg_BeginCleanUp(n) auto __CleanUpFunc__##n = [&]()
#define mg_EndCleanUp(n) mg::scope_guard __ScopeGuard__##n(__CleanUpFunc__##n);
#define mg_CleanUp(n, ...) mg_BeginCleanUp(n) { __VA_ARGS__; }; mg_EndCleanUp(n)
#define mg_DismissCleanUp(n) { __ScopeGuard__##n.Dismissed = true; }
