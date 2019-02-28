#include <stdio.h>
#include <string.h>
#include "mg_assert.h"
#include "mg_logger.h"
#include "mg_io.h"
#include "mg_utils.h"

namespace mg {

logger& GlobalLogger() {
  static logger Logger;
  return Logger;
}

void SetBufferMode(logger* Logger, buffer_mode Mode) {
  Logger->Mode = Mode;
}

void SetBufferMode(buffer_mode Mode) {
  SetBufferMode(&GlobalLogger(), Mode);
}

FILE* GetFileHandle(logger* Logger, cstr FileName) {
  int MaxSlots = Size(Logger->FileHandles);
  u32 FullHash = Murmur3_32((const u8*)FileName, strlen(FileName), 37);
  int Index = FullHash % MaxSlots;
  FILE** Fp = &Logger->FileHandles[Index];
  bool Collision = false;
  if (*Fp && (Logger->FileNameHashes[Index] != FullHash ||
              strncmp(FileName, Logger->FileNames[Index], 64) != 0))
    Collision = true;

  if (Collision) { // collision, find the next empty slot
    for (int I = 0; I < MaxSlots; ++I) {
      int J = (Index + I) % MaxSlots;
      Fp = &Logger->FileHandles[J];
      if (!Logger->FileNames[J] || strncmp(FileName, Logger->FileNames[J], 64) == 0) {
        Index = J;
        break;
      }
    }
  }
  if (!*Fp) { // empty slot, open a new file for logging
    mg_Assert(!Logger->FileNames[Index]);
    mg_Assert(!Logger->FileHandles[Index]);
    *Fp = fopen(FileName, "w");
    mg_AbortIf(!*Fp, "File %s cannot be created", FileName);
    if (Logger->Mode == buffer_mode::Full) // full buffering
      setvbuf(*Fp, nullptr, _IOFBF, BUFSIZ);
    else if (Logger->Mode == buffer_mode::Line) // line buffering (same as full buffering on Windows)
      setvbuf(*Fp, nullptr, _IOLBF, BUFSIZ);
    else // no buffering
      setvbuf(*Fp, nullptr, _IONBF, 0);
    Logger->FileNames[Index] = FileName;
    Logger->FileNameHashes[Index] = FullHash;
  } else if (Collision) { // no more open slots
    mg_Abort("No more logger slots");
  }
  return *Fp;
}

} // namespace mg
