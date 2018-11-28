/* Command-line argument processing */
#pragma once

#include "mg_types.h"

namespace mg {

cstr GetOptionValue(int NumArgs, cstr* Args, cstr Option);
bool OptionExists(int NumArgs, cstr* Args, cstr Option);

} // namespace mg
