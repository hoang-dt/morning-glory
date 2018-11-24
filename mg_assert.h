/* Assert macros that carry file and line information, as well as a custom message */

#pragma once

#define mg_Assert(Cond)
#define mg_AssertMsg(Cond, Msg)
#define mg_AssertFmt(Cond, Fmt, ...)

#include "mg_assert.inl"
