#pragma once

/* Avoid compiler warning about unused variable */
#define mg_Unused(X) do { (void)sizeof(X); } while(0)

/* Return the number of elements in a static array */
#define mg_ArraySize(X) int(sizeof(X) / sizeof(X[0]))

/* Return the number of comma-separated arguments */
#define mg_NumArgs(...) (mg::Count(#__VA_ARGS__, ',') + 1)

/* 2-level stringify */
#define mg_Str(...) mg_StrHelper(__VA_ARGS__)
#define mg_StrHelper(...) #__VA_ARGS__

/* 2-level concat */
#define mg_Cat(A, ...) mg_CatHelper(A, __VA_ARGS__)
#define mg_CatHelper(A, ...) A ## __VA_ARGS__

#define mg_FPrintHelper(...)\
  __VA_OPT__(fprintf(stderr, __VA_ARGS__))

#define mg_SPrintHelper(Buf, L, ...)\
  __VA_OPT__(snprintf(Buf + L, sizeof(Buf) - size_t(L), __VA_ARGS__));\
  mg_Unused(L)

#define mg_ExtractFirst(X, ...) X

#define mg_BitSizeOf(X) ((int)sizeof(X) * 8)

#define mg_Restrict

#define mg_Inline

/* Short for template <typename ...> which sometimes can get too verbose */
#define mg_T(t) template <typename t>
#define mg_T2(t, u) template <typename t, typename u>
#define mg_Ti(t) mg_T(t) mg_Inline
#define mg_T2i(t, u) mg_T2(t, u) mg_Inline

#include "mg_macros.inl"
