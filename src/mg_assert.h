/* Assert macros that carry file and line information, as well as a custom message */

#pragma once

// TODO: add a mg_AssertIf
#define mg_Assert(Cond, ...)
#define mg_AbortIf(Cond, ...)
#define mg_Abort(...)

namespace mg {

using handler = void (int);
void AbortHandler(int Signum);
void SetHandleAbortSignals(handler& Handler = AbortHandler);

} // namespace mg

#include "mg_assert.inl"
