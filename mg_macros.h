#pragma once

/* Return the number of elements in a static array */
#define mg_ArraySize(x) sizeof(x) / sizeof(*(x))

/* Return the number of comma-separated arguments */
#define mg_NumArgs(...) (mg::CountOccurrences(#__VA_ARGS__, ',') + 1)

#include "mg_macros.inl"
