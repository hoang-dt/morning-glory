/* Command-line argument processing */
#pragma once

#include "mg_types.h"

namespace mg {

bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, cstr* Value);
bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, int* Value);
bool OptionExists(int NumArgs, cstr* Args, cstr Option);

} // namespace mg
