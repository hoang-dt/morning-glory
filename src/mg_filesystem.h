#pragma once

#include "mg_string.h"

namespace mg {

/* Only support the forward slash '/' separator. */
struct path {
  constexpr static int NPartsMax = 64;
  stref Parts[NPartsMax] = {}; /* e.g. home, dir, file.txt */
  int NParts = 0;
  path();
  path(stref Str);
};

void Init(path* Path, stref Str);
/* Add a part to the end (e.g. "C:/Users" + "Meow" = "C:/Users/Meow"). */
void Append(path* Path, stref Component);
/* Remove the last part, e.g., removing the file name at the end of a path. */
stref GetFileName(stref Path);
stref GetDirName(stref Path);
str ToString(const path& Path);
/* Get the directory where the program is launched from. */
bool IsRelative(stref Path);
bool CreateFullDir(stref Path);
bool DirExists(stref Path);

} // namespace mg
