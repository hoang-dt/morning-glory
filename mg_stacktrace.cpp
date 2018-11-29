#if defined(_WIN32)
// Adapted from
// http://www.rioki.org/2017/01/09/windows_stacktrace.html and
// http://blog.aaronballman.com/2011/04/generating-a-stack-crawl/
#define WIN32_LEAN_AND_MEAN
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
        mg_Print(Pr, "%s, line %lu: %.*s\n", Line.FileName, Line.LineNumber, (int)Info->NameLen, Info->Name);
      } else if (GetModuleFileNameA((HINSTANCE)Info->ModBase, ModBuf, MAX_PATH)) {
        mg_Print(Pr, "%s: %.*s\n", ModBuf, (int)Info->NameLen, Info->Name);
      } else {
        mg_Print(Pr, "%.*s\n", (int)Info->NameLen, Info->Name);
      }
    }
  }
  return SymCleanup(Proc);
}
} // namespace mg
#elif defined(__linux__) || defined(__APPLE__)
// Adapted from
// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "mg_io.h"
#include "mg_macros.h"
namespace mg {
// TODO: get the line number (add2line)
bool PrintStacktrace(printer* Pr) {
  mg_Print(Pr, "Stack trace:\n");
  constexpr int MaxFrames = 63;
  void* AddrList[MaxFrames + 1]; // Storage array for stack trace address data
  /* Retrieve current stack addresses */
  int AddrLen = backtrace(AddrList, sizeof(AddrList) / sizeof(void*));
  if (AddrLen == 0) {
    mg_Print(Pr, "  <empty, possibly corrupt>\n");
    return false;
  }
  /* Resolve addresses into strings containing "filename(function+address)", */
  char** SymbolList = backtrace_symbols(AddrList, AddrLen);
  size_t FuncNameSize = 128;
  char Buffer[128];
  char* FuncName = Buffer;
  for (int I = 1; I < AddrLen; ++I) { // iterate over the returned symbol lines (skip the first)
	  char* BeginName = 0, *BeginOffset = 0, *EndOffset = 0;
    /* Find parentheses and +address offset surrounding the mangled name:
    e.g., ./module(function+0x15c) [0x8048a6d] */
    for (char* P = SymbolList[I]; *P; ++P) {
      if (*P == '(')
        BeginName = P;
      else if (*P == '+')
        BeginOffset = P;
      else if (*P == ')' && BeginOffset) {
        EndOffset = P;
        break;
      }
    }
    if (BeginName && BeginOffset && EndOffset && BeginName < BeginOffset) {
      *BeginName++ = '\0';
	    *BeginOffset++ = '\0';
	    *EndOffset = '\0';
	    /* mangled name is now in [BeginName, BeginOffset) and caller offset in [BeginOffset, EndOffset) */
	    int Status;
	    char* Ret = abi::__cxa_demangle(BeginName, FuncName, &FuncNameSize, &Status);
	    if (Status == 0) {
		    FuncName = Ret; // use possibly realloc()-ed string
		    mg_Print(Pr, "  %s : %s+%s\n", SymbolList[I], FuncName, BeginOffset);
	    } else { // demangling failed
		    mg_Print(Pr, "  %s : %s()+%s\n", SymbolList[I], BeginName, BeginOffset);
	    }
    } else { // couldn't parse the line? print the whole line.
      mg_Print(Pr, "  %s\n", SymbolList[I]);
    }
  }
  free(SymbolList);
  return true;
}
} // namespace mg
#endif
