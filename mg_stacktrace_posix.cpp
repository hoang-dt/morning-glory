// Adapted from
// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0

#include "mg_stacktrace.h"

#if defined __linux__ || defined __APPLE__

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "mg_io.h"
#include "mg_macros.h"

namespace mg {

void PrintStacktrace(char* Buf, int Size) {
  printer Pr(Buf, Size);
  mg_Print(&Pr, "Stack trace:\n");
  constexpr int MaxFrames = 63;
  void* AddrList[MaxFrames + 1]; // Storage array for stack trace address data
  /* Retrieve current stack addresses */
  int AddrLen = backtrace(AddrList, sizeof(AddrList) / sizeof(void*));
  if (AddrLen == 0) {
    mg_Print(&Pr, "  <empty, possibly corrupt>\n");
    return;
  }
  /* Resolve addresses into strings containing "filename(function+address)", */
  char** SymbolList = backtrace_symbols(AddrList, AddrLen);
  /* Allocate string which will be filled with the demangled function name */
  size_t FuncNameSize = 64;
  char* FuncName = (char*)malloc(FuncNameSize);
  for (int I = 1; I < AddrLen; I++) { // iterate over the returned symbol lines (skip the first)
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
		    mg_PrintFmt(&Pr, "  %s : %s+%s\n", SymbolList[I], FuncName, BeginOffset);
	    } else { // demangling failed
		    mg_PrintFmt(&Pr, "  %s : %s()+%s\n", SymbolList[I], BeginName, BeginOffset);
	    }
    } else { // couldn't parse the line? print the whole line.
      mg_PrintFmt(&Pr, "  %s\n", SymbolList[I]);
    }
  }
  free(FuncName);
  free(SymbolList);
}

} // namespace mg

#endif
