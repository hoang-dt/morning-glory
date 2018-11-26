#include "mg_assert.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_types.h"

namespace mg {

printer::printer(char* Buf, int Size) : Buf(Buf), Size(Size), File(nullptr) {}
printer::printer(FILE* File) : Buf(nullptr), Size(0), File(File) {}

void Reset(printer* Pr, char* Buf, int Size) {
  Pr->Buf = Buf;
  Pr->Size = Size;
  Pr->File = nullptr;
}

void Reset(printer* Pr, FILE* File) {
  Pr->Buf = nullptr;
  Pr->Size = 0;
  Pr->File = File;
}

error ReadFile(cstr Fname, buffer* Buf) {
  mg_Assert((Buf->Data && Buf->Size) || (!Buf->Data && !Buf->Size));

  FILE* Fp = fopen(Fname, "rb");
  mg_BeginCleanUp(0) { if (Fp) fclose(Fp); }; mg_EndCleanUp(0)
  if (!Fp) { return mg_ErrorMsg(FileOpenFailed, Fname); }

  /* Determine the file size */
  if (mg_FSeek(Fp, 0, SEEK_END)) return mg_ErrorMsg(FileSeekFailed, Fname);
  size_t Size = 0;
  if ((Size = mg_FTell(Fp)) == size_t(-1)) return mg_ErrorMsg(FileTellFailed, Fname);
  if (mg_FSeek(Fp, 0, SEEK_SET)) return mg_ErrorMsg(FileSeekFailed, Fname);
  if (Buf->Size < Size)
    if (!mg_Allocate(Buf->Data, Size)) return mg_ErrorMsg(OutOfMemory, Fname);

  /* Read file contents */
  mg_BeginCleanUp(1) { mg_Deallocate(Buf->Data); }; mg_EndCleanUp(1)
  if (fread(Buf->Data, Size, 1, Fp) != 1) return mg_ErrorMsg(FileReadFailed, Fname);
  Buf->Size = Size;

  mg_DismissCleanUp(1);
  return mg_Error(NoError);
}

} // namespace mg
