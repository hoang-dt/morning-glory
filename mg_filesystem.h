#pragma once

#include "mg_string.h"
#include "mg_types.h"

namespace mg {

/* Only support the forward slash '/' separator. */
struct path {
  constexpr static int NumComponentsMax = 64;
  string_ref Components[NumComponentsMax] = {}; /* e.g. home, dir, file.txt */
  int NumComponents = 0;
  path();
  path(string_ref Str);
};

void Init(path* Path, string_ref Str);
/* Add a component to the end (e.g. "C:/Users" + "Meow" = "C:/Users/Meow"). */
void Append(path* Path, string_ref Component);
/* Remove the last component, useful for removing the file name at the end of a path. */
string_ref GetFileName(string_ref Path);
string_ref GetDirName(string_ref Path);
str ToString(const path& Path);
/* Get the directory where the program is launched from. */
bool IsRelativePath(string_ref Path);
bool CreateFullDir(string_ref Path);
bool DirExists(string_ref Path);

} // namespace mg
