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

#define mg_FPrintHelper(Stream, ...)\
  __VA_OPT__(fprintf(Stream, __VA_ARGS__))

#define mg_SPrintHelper(Buf, L, ...)\
  __VA_OPT__(snprintf(Buf + L, sizeof(Buf) - size_t(L), __VA_ARGS__));\
  mg_Unused(L)

#define mg_ExtractFirst(X, ...) X

#define mg_BitSizeOf(X) ((int)sizeof(X) * 8)

#define mg_Restrict

#define mg_Inline

/* Short for template <typename ...> which sometimes can get too verbose */
#define mg_T(...) template <__VA_OPT__(typename) __VA_ARGS__>
#define mg_T2(t, u) template <typename t, typename u>
#define mg_Ti(t) mg_T(t) mg_Inline
#define mg_T2i(t, u) mg_T2(t, u) mg_Inline

/* Print binary */
#define mg_BinPattern8 "%c%c%c%c%c%c%c%c"
#define mg_BinPattern16 mg_BinPattern8  mg_BinPattern8
#define mg_BinPattern32 mg_BinPattern16 mg_BinPattern16
#define mg_BinPattern64 mg_BinPattern32 mg_BinPattern32
#define mg_BinaryByte(Byte) \
  (Byte & 0x80 ? '1' : '0'),\
  (Byte & 0x40 ? '1' : '0'),\
  (Byte & 0x20 ? '1' : '0'),\
  (Byte & 0x10 ? '1' : '0'),\
  (Byte & 0x08 ? '1' : '0'),\
  (Byte & 0x04 ? '1' : '0'),\
  (Byte & 0x02 ? '1' : '0'),\
  (Byte & 0x01 ? '1' : '0') 
#define mg_BinaryByte64(Val)\
  mg_BinaryByte((Val) >> 56), mg_BinaryByte((Val) >> 48),\
  mg_BinaryByte((Val) >> 40), mg_BinaryByte((Val) >> 32), mg_BinaryByte((Val) >> 24),\
  mg_BinaryByte((Val) >> 16), mg_BinaryByte((Val) >>  8), mg_BinaryByte((Val))

#include "mg_macros.inl"
