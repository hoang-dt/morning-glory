#pragma once

#include "mg_types.h"

namespace mg {

/* Support only either reading or writing, not both at the same time */
struct bit_stream {
  buffer Stream;
  byte* BitPtr; // Pointer to current byte
  u64 BitBuf = 0; // last 64 bits we read
  int BitPos = 0; // how many of those bits we've consumed/written

  inline static array<u64, 65> Masks = []() {
    array<u64, 65> Masks;
    for (int I = 0; I < 64; ++I)
      Masks[I] = (1UL << I) - 1;
    Masks[64] = ~0;
    return Masks;
  }();
};

void Rewind(bit_stream* Bs);
size_t Size(bit_stream* Bs);
/* ---------------- Read functions ---------------- */
void InitRead(bit_stream* Bs, buffer* Stream = nullptr);
/* Refill our buffer */
void Refill(bit_stream* Bs);
/* Peek "count" bits, count must be at most 64 - BitPos */
u64 Peek(bit_stream* Bs, int Count);
/* Consume "count" bits, count must be at most 64 - 7 */
void Consume(bit_stream* Bs, int Count);
/* Extract "count" bits from the stream, count must be at most 64 - 7 */
u64 Read(bit_stream* Bs, int Count = 1);

/* ---------------- Write functions ---------------- */
void InitWrite(bit_stream* Bs, size_t Bytes, allocator* Alloc = &Mallocator());
/* Flush the written bits in our buffer */
void Flush(bit_stream* Bs);
/* Put "count" bits into the buffer, count must be at most 64 - BitPos */
void Put(bit_stream* Bs, u64 N, int Count);
/* Write "count" bits into the stream, count must be at most 64 - 7 */
void Write(bit_stream* Bs, u64 N, int Count = 1);
/* Write "count" bits into the stream, count has no restriction */
void RepeatedWrite(bit_stream* Bs, bool B, int Count);

} // namespace mg

#include "mg_bitstream.inl"
