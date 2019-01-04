#include "mg_assert.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_types.h"

namespace mg {

printer::printer() = default;
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

error ReadFile(cstr FileName, buffer* Buf) {
  mg_Assert((Buf->Data && Buf->Bytes) || (!Buf->Data && !Buf->Bytes));

  FILE* Fp = fopen(FileName, "rb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  if (!Fp)
    return mg_Error(FileOpenFailed, "%s", FileName);

  /* Determine the file size */
  if (mg_FSeek(Fp, 0, SEEK_END))
    return mg_Error(FileSeekFailed, "%s", FileName);
  size_t Size = 0;
  if ((Size = mg_FTell(Fp)) == size_t(-1))
    return mg_Error(FileTellFailed, "%s", FileName);
  if (mg_FSeek(Fp, 0, SEEK_SET))
    return mg_Error(FileSeekFailed, "%s", FileName);
  if ((size_t)Buf->Bytes < Size)
    AllocateBuffer(Buf, Size);

  /* Read file contents */
  mg_CleanUp(1, DeallocateBuffer(Buf));
  if (fread(Buf->Data, Size, 1, Fp) != 1)
    return mg_Error(FileReadFailed, "%s", FileName);

  mg_DismissCleanUp(1);
  return mg_Error(NoError);
}

error WriteFile(cstr FileName, const buffer& Buf) {
  mg_Assert(Buf.Data && Buf.Bytes);

  FILE* Fp = fopen(FileName, "wb");
  mg_CleanUp(0, if (Fp) fclose(Fp));
  if (!Fp)
    return mg_Error(FileCreateFailed, "%s", FileName);

  /* Read file contents */
  if (fwrite(Buf.Data, Buf.Bytes, 1, Fp) != 1)
    return mg_Error(FileWriteFailed, "%s", FileName);
  return mg_Error(NoError);
}

} // namespace mg
