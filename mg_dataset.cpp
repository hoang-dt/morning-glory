#include "mg_dataset.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_scopeguard.h"
#include "mg_string.h"
#include "mg_types.h"
#include "mg_stacktrace.h"

namespace mg {

cstr ToString(metadata& Meta) {
  printer Pr(Meta.String, sizeof(Meta.String));
  mg_Print(&Pr, "file = %s\n", Meta.File);
  mg_Print(&Pr, "name = %s\n", Meta.Name);
  mg_Print(&Pr, "field = %s\n", Meta.Field);
  mg_Print(&Pr, "dimensions = %d %d %d\n", Meta.Dims.X, Meta.Dims.Y, Meta.Dims.Z);
  string_ref TypeStr = ToString(Meta.DataType);
  mg_Print(&Pr, "data type = %.*s", TypeStr.Size, TypeStr.Ptr);
  return Meta.String;
}

error ReadMetadata(cstr Fname, metadata* Meta) {
  buffer Buf;
  error Err = ReadFile(Fname, &Buf);
  if (!Err) return Err;
  mg_BeginCleanUp(0) { mg_Deallocate(Buf.Data); }; mg_EndCleanUp(0)
  string_ref Str((cstr)Buf.Data, (int)Buf.Size);
  tokenizer TkLine(Str, "\r\n");
  for (string_ref Line = Next(&TkLine); Line; Line = Next(&TkLine)) {
    tokenizer TkEq(Line, "=");
    string_ref Attr = Trim(Next(&TkEq));
    string_ref Value = Trim(Next(&TkEq));
    if (!Attr || !Value)
      return mg_ErrorFmt(ParseFailed, "File %s", Fname);

    if (Attr == "file") {
      Copy(mg_StringRef(Meta->File), Trim(Value));
    } else if (Attr == "name") {
      Copy(mg_StringRef(Meta->Name), Trim(Value));
    } else if (Attr == "field") {
      Copy(mg_StringRef(Meta->Field), Trim(Value));
    } else if (Attr == "dimensions") {
      tokenizer TkSpace(Value, " ");
      int D = 0;
      for (string_ref Dim = Next(&TkSpace); Dim && D < 4; Dim = Next(&TkSpace), ++D)
        if (!ToInt(Dim, &Meta->Dimensions[D]))
          return mg_ErrorFmt(ParseFailed, "File %s", Fname);
      if (D >= 4) return mg_ErrorFmt(DimensionsTooMany, "File %s", Fname);
      if (D <= 2) Meta->Dimensions[2] = 1;
      if (D <= 1) Meta->Dimensions[1] = 1;
    } else if (Attr == "data type") {
      if (!(Meta->DataType = data_type(Value))) {
        return mg_ErrorFmt(TypeNotSupported, "File %s", Fname);
      }
    } else {
      return mg_ErrorFmt(AttributeNotFound, "File %s", Fname);
    }
  }
  return mg_Error(NoError);
}

} // namespace mg
