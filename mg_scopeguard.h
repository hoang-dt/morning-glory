#pragma once

#include "mg_error.h"

namespace mg {

template <typename func_t>
struct scope_guard {
  func_t Func;
  bool Dismissed = false;
  scope_guard(const func_t& Func) : Func(Func) {}
  ~scope_guard() { if (!Dismissed) { Func(); } }
};

#define mg_BeginCleanUp(n) auto __CleanUpFunc__##n = [&]()
#define mg_EndCleanUp(n) mg::scope_guard __ScopeGuard__##n(__CleanUpFunc__##n);
#define mg_Dismiss(n) { __ScopeGuard__##n.Dismissed = true; }

} // namespace mg
