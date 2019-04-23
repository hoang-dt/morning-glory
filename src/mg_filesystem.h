#pragma once

#include "mg_string.h"
#include "mg_types.h"

namespace mg {

/* Only support the forward slash '/' separator. */
struct path {
  constexpr static int NumComponentsMax = 64;
  str_ref Components[NumComponentsMax] = {}; /* e.g. home, dir, file.txt */
  int NumComponents = 0;
  path();
  path(str_ref Str);
};

void Init(path* Path, str_ref Str);
/* Add a component to the end (e.g. "C:/Users" + "Meow" = "C:/Users/Meow"). */
void Append(path* Path, str_ref Component);
/* Remove the last component, useful for removing the file name at the end of a path. */
str_ref GetFileName(str_ref Path);
str_ref GetDirName(str_ref Path);
str ToString(const path& Path);
/* Get the directory where the program is launched from. */
bool IsRelativePath(str_ref Path);
bool CreateFullDir(str_ref Path);
bool DirExists(str_ref Path);

} // namespace mg
