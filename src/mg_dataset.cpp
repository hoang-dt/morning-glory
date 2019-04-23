#include "mg_dataset.h"
#include "mg_error.h"
#include "mg_filesystem.h"
#include "mg_io.h"
#include "mg_scopeguard.h"
#include "mg_string.h"
#include "mg_types.h"
#include "mg_stacktrace.h"
#include <stdio.h>
#include <ctype.h>

namespace mg {

cstr ToString(const metadata& Meta) {
  printer Pr(Meta.String, sizeof(Meta.String));
  mg_Print(&Pr, "file = %s\n", Meta.File);
  mg_Print(&Pr, "name = %s\n", Meta.Name);
  mg_Print(&Pr, "field = %s\n", Meta.Field);
  mg_Print(&Pr, "dimensions = %d %d %d\n", Meta.Dims.X, Meta.Dims.Y, Meta.Dims.Z);
  str_ref TypeStr = ToString(Meta.Type);
  mg_Print(&Pr, "data type = %.*s", TypeStr.Size, TypeStr.Ptr);
  return Meta.String;
}

/* MIRANDA-DENSITY-[96-96-96]-Float64.raw */
error<> ParseMeta(str_ref FilePath, metadata* Meta) {
  str_ref FileName = GetFileName(FilePath);
  char Type[8];
  if (6 == sscanf(FileName.ConstPtr, "%[^-]-%[^-]-[%d-%d-%d]-%[^.]", Meta->Name,
                  Meta->Field, &Meta->Dims.X, &Meta->Dims.Y, &Meta->Dims.Z, Type))
  {
    Type[0] = tolower(Type[0]);
    Meta->Type = StringTo<data_type>()(str_ref(Type));
    Copy(mg_StringRef(Meta->File), FilePath);
    return mg_Error(err_code::NoError);
  }
  return mg_Error(err_code::ParseFailed);
}

/* file = MIRANDA-DENSITY-[96-96-96]-Float64.raw
 * name = MIRANDA
 * field = DATA
 * dimensions = 96 96 96
 * type = float64
 */
error<> ReadMeta(cstr FileName, metadata* Meta) {
  buffer Buf;
  error Ok = ReadFile(FileName, &Buf);
  if (Ok.Code != err_code::NoError) return Ok;
  mg_CleanUp(0, DeallocBuf(&Buf));
  str_ref Str((cstr)Buf.Data, (int)Buf.Bytes);
  tokenizer TkLine(Str, "\r\n");
  for (str_ref Line = Next(&TkLine); Line; Line = Next(&TkLine)) {
    tokenizer TkEq(Line, "=");
    str_ref Attr = Trim(Next(&TkEq));
    str_ref Value = Trim(Next(&TkEq));
    if (!Attr || !Value)
      return mg_Error(err_code::ParseFailed, "File %s", FileName);

    if (Attr == "file") {
      path Path(GetDirName(FileName));
      Append(&Path, Trim(Value));
      Copy(mg_StringRef(Meta->File), ToString(Path));
    } else if (Attr == "name") {
      Copy(mg_StringRef(Meta->Name), Trim(Value));
    } else if (Attr == "field") {
      Copy(mg_StringRef(Meta->Field), Trim(Value));
    } else if (Attr == "dimensions") {
      tokenizer TkSpace(Value, " ");
      int D = 0;
      for (str_ref Dim = Next(&TkSpace); Dim && D < 4;
           Dim = Next(&TkSpace), ++D) {
        if (!ToInt(Dim, &Meta->Dims[D]))
          return mg_Error(err_code::ParseFailed, "File %s", FileName);
      }
      if (D >= 4)
        return mg_Error(err_code::DimensionsTooMany, "File %s", FileName);
      if (D <= 2) Meta->Dims[2] = 1;
      if (D <= 1) Meta->Dims[1] = 1;
    } else if (Attr == "type") {
      if ((Meta->Type = StringTo<data_type>()(Value)) ==
          data_type::__Invalid__)
      {
        return mg_Error(err_code::TypeNotSupported, "File %s", FileName);
      }
    } else {
      return mg_Error(err_code::AttributeNotFound, "File %s", FileName);
    }
  }
  return mg_Error(err_code::NoError);
}

} // namespace mg
