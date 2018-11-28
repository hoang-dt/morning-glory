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
#define Before(x)\
  After(x)

void testest() {
  int x = -1;
  #define After(x) printf("%d\n", x);
  Before(x);
}

mg_Enum(errors, int, error1=0, error2=1)

void print_clean0() {
  puts("cleaning up 0\n");
}

void print_clean1() {
  puts("cleaning up 1\n");
}

error C() {
  int a = 10;
  return mg_ErrorMsg(SizeTooSmall, "(%d)", a);
}

error B() {
  error Err = C();
  return mg_PropagateError(Err);
}

error A() {
  error Err = B();
  return mg_PropagateError(Err);
}


int main(int Argc, const char** Argv) {
  int T = 0;
  mg_Assert(2 > 3, "hello%d", T);
  cstr Value = GetOptionValue(Argc, Argv, "-v");
  if (Value) {
    printf("%s", Value);
  } else {
    printf("no\n");
  }
  int a = 1;
  int b = 2;
  double Sq = SquaredError((double*)&a, (double*)&b, 1, data_type::int32);
  printf("%f\n", Sq);
  testest();
  path P("./test3");
  Append(&P, "cde");
  Append(&P, "acb");
  if (CreateFullDir(ToString(P)))
    puts("yes");
  else
    puts("no");
  // return 0;
  printf(IsEven(4) ? "4 is even" : "false");
  printf(IsOdd(3) ? "3 is odd" : "false");
  error Ok = A();
  if (!Ok) {
    printer Pr(stderr);
    mg_Print(&Pr, "%s\n", ToString(Ok));
    PrintStacktrace(&Pr, Ok);
  }
  // return 0;
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
  auto Err2 = mg_ErrorMsg(SizeMismatched, "Hello %d", N);
  auto Ss = ToString(Err2);
  printf("%s\n", Ss);
  auto Err3 = mg_ErrorMsg(UnknownError, "Hahaha");
  printf("%s\n", ToString(Err3));
  auto Err4 = mg_Error(FileReadFailed);
  printf("%s\n", ToString(Err4));
  mg::v3<int> v3;
  v3[0] = 1;
  v3.X = 2;
  // mg_Assert(false, "Size %d", S.Size);
  mg::buffer Buf;
  auto Error = mg::ReadFile("abc.txt", &Buf);
  if (!Error) {
    fprintf(stderr, "%s\n", ToString(Error));
  } else {
    fprintf(stderr, "%s", Buf.Data);
  }
  mg_DismissCleanUp(0)
}
