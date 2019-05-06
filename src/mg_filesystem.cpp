#include <string.h>
#include "mg_assert.h"
#include "mg_algorithm.h"
#include "mg_filesystem.h"
#include "mg_io.h"

#if defined(_WIN32)
  #include <direct.h>
  #include <io.h>
  #include <Windows.h>
  #define GetCurrentDir _getcwd
  #define MkDir(Dir) _mkdir(Dir)
  #define Access(Dir) _access(Dir, 0)
#elif defined(__linux__) || defined(__APPLE__)
  #include <sys/stat.h>
  #include <unistd.h>
  #define GetCurrentDir getcwd
  #define MkDir(Dir) mkdir(Dir, 0733)
  #define Access(Dir) access(Dir, F_OK)
#endif

namespace mg {

path::
path() = default;

path::
path(stref Str) { Init(this, Str); }

void
Init(path* Path, stref Str) {
  Path->Parts[0] = Str;
  Path->NParts = 1;
}

void Append(path* Path, stref Component) {
  mg_Assert(Path->NParts < Path->NPartsMax, "too many path parts");
  Path->Parts[Path->NParts++] = Component;
}

stref
GetFileName(stref Path) {
  mg_Assert(!Contains(Path, '\\'));
  cstr LastSlash = FindLast(RevBegin(Path), RevEnd(Path), '/');
  if (LastSlash != RevEnd(Path))
    return SubString(Path, int(LastSlash - Begin(Path) + 1),
                     Path.Size - int(LastSlash - Begin(Path)));
  return Path;
}

stref
GetDirName(stref Path) {
  mg_Assert(!Contains(Path, '\\'));
  cstr LastSlash = FindLast(RevBegin(Path), RevEnd(Path), '/');
  if (LastSlash != RevEnd(Path))
    return SubString(Path, 0, int(LastSlash - Begin(Path)));
  return Path;
}

str
ToString(const path& Path) {
  printer Pr(ScratchBuf, sizeof(ScratchBuf));
  for (int I = 0; I < Path.NParts; ++I) {
    mg_Print(&Pr, "%.*s", Path.Parts[I].Size, Path.Parts[I].Ptr);
    if (I + 1 < Path.NParts)
      mg_Print(&Pr, "/");
  }
  return ScratchBuf;
}

bool
IsRelative(stref Path) {
  if (Path.Size > 0 && Path[0] == '/')  // e.g. /usr/local
    return false;
  if (Path.Size > 2 && Path[1] == ':' && Path[2] == '/')  // e.g. C:/Users
    return false;
  return true;
}

bool
CreateFullDir(stref Path) {
  str PathCopy = ToString(Path);
  int Error = 0;
  str P = PathCopy;
  for (P = strchr(PathCopy, '/'); P; P = strchr(P + 1, '/')) {
    *P = '\0';
    Error = MkDir(PathCopy);
    *P = '/';
  }
  Error = MkDir(PathCopy);
  return (Error == 0);
}

bool
DirExists(stref Path) {
  str PathCopy = ToString(Path);
  return Access(PathCopy) == 0;
}

} // namespace mg

#undef GetCurrentDir
#undef MkDir
#undef Access
