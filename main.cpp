#include "mg_enum.h"
#include "mg_error.h"
#include "mg_io.h"
#include "mg_memory.h"

mg_Enum(errors, int, error1, error2)

int main() {
  // mg::error_code2 err = mg::error_code2::NoError;
  //mg::errors err = mg::errors::error1;
  //int n = err.ToString().Size;
  mg::buffer Buf;
  auto Error = mg::ReadFile("abc.txt", &Buf);
  if (!Error) {
    fprintf(stderr, "%s\n", ToString(Error));
  } else {
    fprintf(stderr, "%s", Buf.Data);
  }
}
