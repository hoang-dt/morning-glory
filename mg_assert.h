/* Assert macros that carry file and line information, as well as a custom message */

// TODO: hook into abort signal

#pragma once

#define mg_Assert(Cond, ...)
#define mg_AbortIf(Cond, ...)
#define mg_Abort(...)

#include "mg_assert.inl"
