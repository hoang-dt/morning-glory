#include "mg_assert.h"
#include "mg_dataset.h"
#include "mg_enum.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_math.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"
#include "mg_dataset.h"
#include "mg_wavelet.h"

using namespace mg;

mg_Enum(errors, int, error1=0, error2=1)

void print_clean0() {
  puts("cleaning up 0\n");
}

void print_clean1() {
  puts("cleaning up 1\n");
}

error C() {
  int a = 10;
  return mg_ErrorFmt(SizeTooSmall, "(%d)", a);
}

error B() {
  error Err = C();
  return mg_PropagateError(Err);
}

error A() {
  error Err = B();
  return mg_PropagateError(Err);
}

int main() {
  printf(IsEven(4) ? "4 is even" : "false");
  printf(IsOdd(3) ? "3 is odd" : "false");
  error Ok = A();
  if (!Ok) {
    printer Pr(stderr);
    mg_PrintFmt(&Pr, "%s\n", ToString(Ok));
    PrintStacktrace(&Pr, Ok);
  }
  return 0;
  metadata Meta;
  Ok = ReadMetadata("abc.meta", &Meta);
  if (Ok) {
    puts("No error\n");
    printf("%s\n", ToString(Meta));
  }
  else {
    puts("Error\n");
    auto ErrStr = ToString(Ok);
    printf("%d %s", Ok.Code.Value, ErrStr);
  }
  return 0;
  mg_BeginCleanUp(0) { print_clean0(); }; mg_EndCleanUp(0)
  mg_BeginCleanUp(1) { print_clean1(); }; mg_EndCleanUp(1)
  int N = 1;
  auto Err2 = mg_ErrorFmt(SizeMismatched, "Hello %d", N);
  auto Ss = ToString(Err2);
  printf("%s\n", Ss);
  auto Err3 = mg_ErrorMsg(UnknownError, "Hahaha");
  printf("%s\n", ToString(Err3));
  auto Err4 = mg_Error(FileReadFailed);
  printf("%s\n", ToString(Err4));
  mg::v3<int> v3;
  v3[0] = 1;
  v3.X = 2;
  // mg_AssertMsg(false, "Size %d", S.Size);
  mg::buffer Buf;
  auto Error = mg::ReadFile("abc.txt", &Buf);
  if (!Error) {
    fprintf(stderr, "%s\n", ToString(Error));
  } else {
    fprintf(stderr, "%s", Buf.Data);
  }
  mg_DismissCleanUp(0)
}
