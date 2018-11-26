#if defined(_WIN32)

#include <string>
#include <windows.h>
#include <DbgHelp.h>
#include "mg_io.h"

namespace mg {

bool PrintStacktrace(char* Buf, int Size) {
  printer Pr(Buf, Size);
  mg_Print(&Pr, "Stack trace:\n");
  /* Set up the symbol options */
  ::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME);
  if (!::SymInitialize(::GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols", true))
    return false;
  PVOID Addrs[64] = { 0 }; // Capture 64 stack frames at maximum
  USHORT Frames = CaptureStackBackTrace(1, 64, Addrs, nullptr);
  for (USHORT I = 0; I < Frames; I++) {
    ULONG64 Buffer[(sizeof(SYMBOL_INFO) + 1024 + sizeof(ULONG64) - 1) / sizeof(ULONG64)] = { 0 };
    SYMBOL_INFO *Info = (SYMBOL_INFO *)Buffer;
    Info->SizeOfStruct = sizeof(SYMBOL_INFO);
    Info->MaxNameLen = 1024;
    DWORD64 Displacement = 0;
    if (::SymFromAddr(::GetCurrentProcess(), (DWORD64)Addrs[I], &Displacement, Info))
      mg_PrintFmt(&Pr, "%.*s\n", (int)Info->NameLen, Info->Name);
  }
  ::SymCleanup(::GetCurrentProcess());
  return true;
}

} // namespace mg

#endif
