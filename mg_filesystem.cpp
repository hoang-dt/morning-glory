#include <string.h>
#include "mg_assert.h"
#include "mg_algorithm.h"
#include "mg_filesystem.h"
#include "mg_io.h"
#include "mg_memory.h"

#if defined(_WIN32)
  #include <direct.h>
  #include <io.h>
  #include <windows.h>
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

path::path() = default;

path::path(str_ref Str) {
  Init(this, Str);
}

void Init(path* Path, str_ref Str) {
  Path->Components[0] = Str;
  Path->NumComponents = 1;
}

void Append(path* Path, str_ref Component) {
  mg_Assert(Path->NumComponents < Path->NumComponentsMax, "too many path components");
  Path->Components[Path->NumComponents++] = Component;
}

str_ref GetFileName(str_ref Path) {
  mg_Assert(!Contains(Path, '\\'));
  cstr LastSlash = FindLast(ConstReverseBegin(Path), ConstReverseEnd(Path), '/');
  if (LastSlash != ConstReverseEnd(Path))
    return SubString(Path, LastSlash - ConstBegin(Path) + 1,
                     Path.Size - (LastSlash - ConstBegin(Path)));
  return Path;
}

str_ref GetDirName(str_ref Path) {
  mg_Assert(!Contains(Path, '\\'));
  cstr LastSlash = FindLast(ConstReverseBegin(Path), ConstReverseEnd(Path), '/');
  if (LastSlash != ConstReverseEnd(Path))
    return SubString(Path, 0, LastSlash - ConstBegin(Path));
  return Path;
}

str ToString(const path& Path) {
  printer Pr(ScratchBuf, sizeof(ScratchBuf));
  for (int I = 0; I < Path.NumComponents; ++I) {
    mg_Print(&Pr, "%.*s", Path.Components[I].Size, Path.Components[I].Ptr);
    if (I + 1 < Path.NumComponents)
      mg_Print(&Pr, "/");
  }
  return ScratchBuf;
}

bool IsRelative(str_ref Path) {
  if (Path.Size > 0 && Path[0] == '/')  // e.g. /usr/local
    return false;
  if (Path.Size > 2 && Path[1] == ':' && Path[2] == '/')  // e.g. C:/Users
    return false;
  return true;
}

bool CreateFullDir(str_ref Path) {
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

bool DirExists(str_ref Path) {
  str PathCopy = ToString(Path);
  return Access(PathCopy) == 0;
}

} // namespace mg

#undef GetCurrentDir
#undef MkDir
#undef Access
