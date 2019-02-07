#pragma once

#include "mg_assert.h"
#include "mg_algorithm.h"
#include "mg_macros.h"

namespace mg {

mg_ForceInline void Rewind(bitstream* Bs) {
  Bs->BitPtr = Bs->Stream.Data;
  Bs->BitBuf = Bs->BitPos = 0;
}

mg_ForceInline size_t Size(const bitstream& Bs) {
  return (Bs.BitPtr - Bs.Stream.Data) + (Bs.BitPos + 7) / 8;
}

mg_ForceInline size_t BitSize(const bitstream& Bs) {
  return (Bs.BitPtr - Bs.Stream.Data) + Bs.BitPos;
}

mg_ForceInline void InitRead(bitstream* Bs, buffer Stream) {
  mg_Assert(!Stream.Data || Stream.Bytes > 0);
  Bs->Stream = Stream;
  Rewind(Bs);
  Refill(Bs);
}

mg_ForceInline void Refill(bitstream* Bs) {
  mg_Assert(Bs->BitPos <= 64);
  Bs->BitPtr += Bs->BitPos >> 3; // ignore the bytes we've consumed
  Bs->BitBuf = *(u64*)Bs->BitPtr; // refill
  Bs->BitPos &= 7; // (% 8) left over bits that don't make a full byte
}

mg_ForceInline u64 Peek(bitstream* Bs, int Count) {
  mg_Assert(Count >= 0 && Bs->BitPos + Count <= 64);
  u64 Remaining = Bs->BitBuf >> Bs->BitPos; // the bits we have not consumed
  return Remaining & bitstream::Masks[Count]; // return the bottom count bits
}

mg_ForceInline void Consume(bitstream* Bs, int Count) {
  // mg_Assert(Count >= 0 && Count <= 64 - 7);
  mg_Assert(Count + Bs->BitPos <= 64);
  Bs->BitPos += Count;
}

mg_ForceInline u64 Read(bitstream* Bs, int Count) {
  mg_Assert(Count >= 0 && Count <= 64 - 7);
  if (Count + Bs->BitPos > 64)
    Refill(Bs);
  u64 Result = Peek(Bs, Count);
  Consume(Bs, Count);
  return Result;
}

mg_ForceInline u64 ReadLong(bitstream* Bs, int Count) {
  mg_Assert(Count >= 0 && Count <= 64);
  int FirstBatchCount = Min(Count, 64 - Bs->BitPos);
  u64 Result = Peek(Bs, FirstBatchCount);
  Consume(Bs, FirstBatchCount);
  if (Count > FirstBatchCount) {
    Refill(Bs);
    Result |= Peek(Bs, Count - FirstBatchCount) << FirstBatchCount;
    Consume(Bs, Count - FirstBatchCount);
  }
  return Result;
}

mg_ForceInline void InitWrite(bitstream* Bs, buffer Buf) {
  mg_Assert((size_t)Buf.Bytes >= sizeof(Bs->BitBuf));
  Bs->Stream = Buf;
  Bs->BitPtr = Buf.Data;
  Bs->BitBuf = Bs->BitPos = 0;
}

mg_ForceInline void Flush(bitstream* Bs) {
  mg_Assert(Bs->BitPos <= 64);
  /* write the buffer to memory */
  *(u64*)Bs->BitPtr = Bs->BitBuf; // TODO: make sure this write is in little-endian
  int BytePos = Bs->BitPos >> 3; // number of bytes in the buffer we have used
  /* shift the buffer to the right (the convoluted logic is to avoid shifting by 64 bits) */
  Bs->BitBuf = (Bs->BitBuf >> 1) >> ((BytePos << 3) - 1);
  Bs->BitPtr += BytePos; // advance the pointer
  Bs->BitPos &= 7; // % 8
}

mg_ForceInline void FlushAndMoveToNextByte(bitstream* Bs) {
  Flush(Bs);
  ++Bs->BitPtr;
}

mg_ForceInline void Put(bitstream* Bs, u64 N, int Count) {
  mg_Assert(Count >= 0 && Bs->BitPos + Count <= 64);
  Bs->BitBuf |= (N & bitstream::Masks[Count]) << Bs->BitPos;
  Bs->BitPos += Count;
}

mg_ForceInline u64 Write(bitstream* Bs, u64 N, int Count) {
  mg_Assert(Count >= 0 && Count <= 64 - 7);
  if (Count + Bs->BitPos >= 64)
    Flush(Bs);
  Put(Bs, N, Count);
  return N;
}

mg_ForceInline u64 WriteLong(bitstream* Bs, u64 N, int Count) {
  mg_Assert(Count >= 0 && Count <= 64);
  int FirstBatchCount = Min(Count, 64 - Bs->BitPos);
  Put(Bs, N, FirstBatchCount);
  if (Count > FirstBatchCount) {
    Flush(Bs);
    Put(Bs, N >> FirstBatchCount, Count - FirstBatchCount);
  }
  return N;
}

mg_ForceInline void RepeatedWrite(bitstream* Bs, bool B, int Count) {
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
