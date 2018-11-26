// Adapted from
// http://www.rioki.org/2017/01/09/windows_stacktrace.html and
// http://blog.aaronballman.com/2011/04/generating-a-stack-crawl/
#if defined(_WIN32)

#include <windows.h>
#include <DbgHelp.h>
#include "mg_io.h"
#include "mg_mutex.h"

namespace mg {

static mutex StacktraceMutex;

bool PrintStacktrace(printer* Pr) {
  lock Lck(&StacktraceMutex);
  mg_Print(Pr, "Stack trace:\n");
  SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME);
  HANDLE Proc = GetCurrentProcess();
  if (!SymInitialize(Proc, "http://msdl.microsoft.com/download/symbols", true))
    return false;
  PVOID Addrs[64] = { 0 }; // Capture 64 stack frames at maximum
  USHORT NumFrames = CaptureStackBackTrace(1, 64, Addrs, nullptr);
  for (USHORT I = 0; I < NumFrames; ++I) {
    ULONG64 Buffer[(sizeof(SYMBOL_INFO) + 512 + sizeof(ULONG64) - 1) / sizeof(ULONG64)] = { 0 };
    SYMBOL_INFO* Info = (SYMBOL_INFO*)Buffer;
    Info->SizeOfStruct = sizeof(SYMBOL_INFO);
    Info->MaxNameLen = 512;
    DWORD64 Displacement = 0;
    if (SymFromAddr(Proc, (DWORD64)Addrs[I], &Displacement, Info)) {
      char ModBuf[MAX_PATH];
      DWORD Offset = 0;
      IMAGEHLP_LINE Line;
      Line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
      if (SymGetLineFromAddr(Proc, Info->Address, &Offset, &Line)) {
        mg_PrintFmt(Pr, "%s, line %lu: %.*s\n", Line.FileName, Line.LineNumber, (int)Info->NameLen, Info->Name);
      } else if (GetModuleFileNameA((HINSTANCE)Info->ModBase, ModBuf, MAX_PATH)) {
        mg_PrintFmt(Pr, "%s: %.*s\n", ModBuf, (int)Info->NameLen, Info->Name);
      } else {
        mg_PrintFmt(Pr, "%.*s\n", (int)Info->NameLen, Info->Name);
      }
    }
  }
  return SymCleanup(Proc);
}

} // namespace mg

#endif
