/* Command-line argument processing */
// TODO: encapsulate Argc and Argv into a struct so that we can easily pass it around
#pragma once

#include "mg_common.h"

namespace mg {

bool OptVal   (int NArgs, cstr* Args, cstr Opt, cstr* Val);
bool OptVal   (int NArgs, cstr* Args, cstr Opt, int * Val);
bool OptVal   (int NArgs, cstr* Args, cstr Opt, f64 * Val);
bool OptVal   (int NArgs, cstr* Args, cstr Opt, v3i * Val);
bool OptExists(int NArgs, cstr* Args, cstr Opt);

} // namespace mg
