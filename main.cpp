#include "mg_args.h"
#include "mg_assert.h"
#include "mg_common_types.h"
#include "mg_dataset.h"
#include "mg_enum.h"
#include "mg_error.h"
#include "mg_filesystem.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_signal_processing.h"
#include "mg_dataset.h"
#include "mg_types.h"
#include "mg_wavelet.h"

using namespace mg;

int main(int Argc, const char** Argv) {
  cstr DataFile = nullptr;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--dataset", &DataFile), "Provide --dataset");
  metadata Meta;
  error Ok = ReadMetadata(DataFile, &Meta);
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  buffer BufF;
  Ok = ReadFile(Meta.File, &BufF);
  mg_CleanUp(0, mg_Deallocate(BufF.Data););
  mg_AbortIf(!Ok, "%s", ToString(Ok));
  f64* F = (f64*)BufF.Data;
  i64 Size = (i64)Meta.Dimensions.X * Meta.Dimensions.Y * Meta.Dimensions.Z;
  mg_AbortIf((size_t)Size * SizeOf(Meta.DataType) != BufF.Size, "Size mismatched. Check file: %s", Meta.File);
  int NLevels = 0;
  mg_AbortIf(!GetOptionValue(Argc, Argv, "--nlevels", &NLevels), "Provide --nlevels");
  buffer BufFWav;
  mg_Allocate(BufFWav.Data, BufF.Size);
  f64* FWav = (f64*)BufFWav.Data;
  memcpy(BufFWav.Data, BufF.Data, BufF.Size);
  mg_CleanUp(1, mg_Deallocate(BufFWav.Data););
  for (int I = 0; I < NLevels; ++I) {
    FLiftCdf53X(FWav, Meta.Dimensions, v3l{I, I, I});
    FLiftCdf53Y(FWav, Meta.Dimensions, v3l{I, I, I});
    FLiftCdf53Z(FWav, Meta.Dimensions, v3l{I, I, I});
  }
  for (int I = NLevels - 1; I >= 0; --I) {
    ILiftCdf53Z(FWav, Meta.Dimensions, v3l{I, I, I});
    ILiftCdf53Y(FWav, Meta.Dimensions, v3l{I, I, I});
    ILiftCdf53X(FWav, Meta.Dimensions, v3l{I, I, I});
  }
  f64 Psnr = PSNR(FWav, F, Size);
  printf("Psnr = %f\n", Psnr);
}
