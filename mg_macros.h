#pragma once

/* Avoid compiler warning about unused variable */
#define mg_Unused(X) do { (void)sizeof(X); } while(0)

/* Return the number of elements in a static array */
#define mg_ArraySize(X) int(sizeof(X) / sizeof(X[0]))

/* Return the number of comma-separated arguments */
#define mg_NumArgs(...) (mg::CountOccurrences(#__VA_ARGS__, ',') + 1)

/* 2-level stringify */
#define mg_Str(...) mg_PremitiveStr(__VA_ARGS__)
#define mg_PremitiveStr(...) #__VA_ARGS__

/* 2-level concat */
#define mg_Cat(A, ...) mg_PrimitiveCat(A, __VA_ARGS__)
#define mg_PrimitiveCat(A, ...) A ## __VA_ARGS__

#define mg_Restrict

#define mg_ForceInline

#include "mg_macros.inl"
