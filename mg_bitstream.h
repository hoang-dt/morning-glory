// TODO: this code currently works only on little-endian machine. make sure it works for
// both types of endianess
// TODO: bound checking?
/* A bit stream. LSB bits are written first. Bytes are written in little-endian order. */
#pragma once

#include "mg_types.h"

namespace mg {

/* Support only either reading or writing, not both at the same time */
struct bit_stream {
  buffer Stream;
  byte* BitPtr; // Pointer to current byte
  u64 BitBuf = 0; // buffer
  int BitPos = 0; // how many of those bits we've consumed/written

  inline static array<u64, 65> Masks = []() {
    array<u64, 65> Masks;
    for (int I = 0; I < 64; ++I)
      Masks[I] = (u64(1) << I) - 1;
    Masks[64] = ~u64(0);
    return Masks;
  }();
};

void Rewind(bit_stream* Bs);
size_t Size(bit_stream* Bs);
/* ---------------- Read functions ---------------- */
void InitRead(bit_stream* Bs, buffer* Stream = nullptr);
/* Refill our buffer (replace the consumed bytes with new bytes from memory) */
void Refill(bit_stream* Bs);
/* Peek the next "Count" bits from the buffer without consuming them (Count <= 64 - BitPos).
This is often called after Refill(). */
u64 Peek(bit_stream* Bs, int Count = 1);
/* Consume the next "Count" bits from the buffer (Count <= 64 - 7).
This is often called after Refill() and potentially Peek(). */
void Consume(bit_stream* Bs, int Count = 1);
/* Extract "Count" bits from the stream (Count <= 64 - 7).
This performs at most one Refill() call. The restriction on Count is due to the fact that
Refill() works in units of bytes, so at most 7 already consumed bits can be left over. */
u64 Read(bit_stream* Bs, int Count = 1);
/* Similar to Read() but Count is less restrictive (Count <= 64) */
u64 ReadLong(bit_stream* Bs, int Count);

/* ---------------- Write functions ---------------- */
void InitWrite(bit_stream* Bs, buffer Buf);
/* Flush the written BYTES in our buffer to memory */
void Flush(bit_stream* Bs);
/* Flush and move the pointer to the next byte in memory */
void FlushAndMoveToNextByte(bit_stream* Bs);
/* Put "Count" bits into the buffer (Count <= 64 - BitPos) */
void Put(bit_stream* Bs, u64 N, int Count = 1);
/* Write "Count" bits into the stream (Count <= 64 - 7) */
u64 Write(bit_stream* Bs, u64 N, int Count = 1);
/* Similar to Write() but Count is less restrictive (Count <= 64) */
u64 WriteLong(bit_stream* Bs, u64 N, int Count);
/* Write "Count" bits into the stream (Count >= 0) */
void RepeatedWrite(bit_stream* Bs, bool B, int Count);

} // namespace mg

#include "mg_bitstream.inl"
