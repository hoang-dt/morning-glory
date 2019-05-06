/* Command-line argument processing */
// TODO: encapsulate Argc and Argv into a struct so that we can easily pass it around
#pragma once

#include "mg_common.h"

namespace mg {

bool OptionValue(int NArgs, cstr* Args, cstr Option, cstr* Value);
bool OptionValue(int NArgs, cstr* Args, cstr Option, int* Value);
bool OptionValue(int NArgs, cstr* Args, cstr Option, f64* Value);
bool OptionValue(int NumArgs, cstr* Args, cstr Option, v3i* Value);
bool OptionExists(int NArgs, cstr* Args, cstr Option);

} // namespace mg
