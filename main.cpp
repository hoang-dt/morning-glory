#include "mg_assert.h"
#include "mg_enum.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_memory.h"
#include "mg_scopeguard.h"

mg_Enum(errors, int, error1=0, error2=1)

void print_clean0() {
  puts("cleaning up 0\n");
}

void print_clean1() {
  puts("cleaning up 1\n");
}

int main() {
  mg_BeginCleanUp(0) { print_clean0(); }; mg_EndCleanUp(0)
  mg_BeginCleanUp(1) { print_clean1(); }; mg_EndCleanUp(1)
  mg::errors Err = mg::errors::error1;
  auto S = ToString(Err);
  printf("%.*s", int(S.Size), S.Ptr);
  // mg_AssertMsg(false, "Size %d", S.Size);
  mg::buffer Buf;
  auto Error = mg::ReadFile("abc.txt", &Buf);
  if (!Error) {
    fprintf(stderr, "%s\n", ToString(Error));
  } else {
    fprintf(stderr, "%s", Buf.Data);
  }
  mg_DismissCleanUp(0)
  mg_DismissCleanUp(1)
}
