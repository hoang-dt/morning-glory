#pragma once

#include "mg_assert.h"
#include "mg_memory.h"

namespace mg {

inline void Rewind(bit_stream* Bs) {
  Bs->BitPtr = Bs->Stream.Data;
  Bs->BitBuf = Bs->BitPos = 0;
}

inline size_t Size(bit_stream* Bs) {
  return Bs->BitPtr - Bs->Stream.Data;
}

inline void InitRead(bit_stream* Bs, buffer* Stream) {
  if (Stream) {
    mg_Assert(!Stream->Data || Stream->Bytes > 0);
    Bs->Stream = *Stream;
  }
  Rewind(Bs);
  Refill(Bs);
}

inline void Refill(bit_stream* Bs) {
  mg_Assert(Bs->BitPos <= 64);
  Bs->BitPtr += Bs->BitPos >> 3; // ignore the bytes we've consumed
  Bs->BitBuf = *(u64*)Bs->BitPtr; // refill
  Bs->BitPos &= 7; // left over bits that don't make a full byte
}

inline u64 Peek(bit_stream* Bs, int Count) {
  mg_Assert(Count >= 0 && Bs->BitPos + Count <= 64);
  u64 Remaining = Bs->BitBuf >> Bs->BitPos; // shift out the bits we've consumed
  return Remaining & bit_stream::Masks[Count]; // return the bottom count bits
}

inline void Consume(bit_stream* Bs, int Count) {
  mg_Assert(Count >= 0 && Count <= 64 - 7);
  Bs->BitPos += Count;
}

inline u64 Read(bit_stream* Bs, int Count) {
  mg_Assert(Count >= 0 && Count <= 64 - 7);
  if (Count + Bs->BitPos > 64)
    Refill(Bs);
  u64 Result = Peek(Bs, Count);
  Bs->BitPos += Count;
  return Result;
}

inline void InitWrite(bit_stream* Bs, size_t Bytes, allocator* Alloc) {
  mg_Assert(Bytes >= sizeof(Bs->BitBuf));
  Alloc->Allocate(&Bs->Stream, Bytes);
  Bs->BitPtr = Bs->Stream.Data;
  Bs->BitBuf = Bs->BitPos = 0;
}

inline void Flush(bit_stream* Bs) {
  mg_Assert(Bs->BitPos <= 64);
  *(u64*)Bs->BitPtr = Bs->BitBuf;
  int BytePos = Bs->BitPos >> 3;
  Bs->BitBuf = (Bs->BitBuf >> 1) >> ((BytePos << 3) - 1);
  Bs->BitPtr += BytePos;
  Bs->BitPos &= 7;
}

inline void Put(bit_stream* Bs, u64 N, int Count) {
  mg_Assert(Count >= 0 && Bs->BitPos + Count <= 64);
  Bs->BitBuf |= (N & bit_stream::Masks[Count]) << Bs->BitPos;
  Bs->BitPos += Count;
}

inline u64 Write(bit_stream* Bs, u64 N, int Count) {
  mg_Assert(Count >= 0 && Count <= 64 - 7);
  if (Count + Bs->BitPos > 64)
    Flush(Bs);
  Put(Bs, N, Count);
  return N;
}

inline void RepeatedWrite(bit_stream* Bs, bool B, int Count) {
  mg_Assert(Count >= 0);
  u64 N = ~(u64(B) - 1);
  if (Count <= 64 - 7) { // write at most 57 bits
    Write(Bs, N, Count);
  } else { // write more than 57 bits
    while (true) {
      int NBits = 64 - Bs->BitPos;
      if (NBits <= Count) {
        Put(Bs, N, NBits);
        Count -= NBits;
        Flush(Bs);
      } else {
        Put(Bs, N, Count);
        break;
      }
    }
  }
}

} // namespace mg
