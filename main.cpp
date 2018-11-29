#include "mg_args.h"
#include "mg_assert.h"
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
#include "mg_wavelet.h"

using namespace mg;

int main(int Argc, const char** Argv) {
  cstr DataFile = GetOptionValue(Argc, Argv, "--dataset");
  mg_AbortIf(!DataFile, "Provide --dataset");
  metadata Meta;
  error Ok = ReadMetadata(DataFile, &Meta);
  mg_AbortIf(!Ok, "%s", ToString(Ok));

}
