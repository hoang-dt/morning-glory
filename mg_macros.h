#pragma once

/* Avoid compiler warning about unused variable */
#define mg_Unused(x) do { (void)sizeof(x); } while(0)

/* Return the number of elements in a static array */
#define mg_ArraySize(x) int(sizeof(x) / sizeof(x[0]))

/* Return the number of comma-separated arguments */
#define mg_NumArgs(...) (mg::CountOccurrences(#__VA_ARGS__, ',') + 1)

#define mg_Restrict

#include "mg_macros.inl"
