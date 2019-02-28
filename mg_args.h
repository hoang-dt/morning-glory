/* Command-line argument processing */
// TODO: encapsulate Argc and Argv into a struct so that we can easily pass it around
#pragma once

#include "mg_types.h"

namespace mg {

bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, cstr* Value);
bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, int* Value);
bool GetOptionValue(int NumArgs, cstr* Args, cstr Option, f64* Value);
bool OptionExists(int NumArgs, cstr* Args, cstr Option);

} // namespace mg
