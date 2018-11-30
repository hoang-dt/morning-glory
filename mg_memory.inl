#pragma once

#include <stdlib.h>
#include "mg_assert.h"
#include "mg_types.h"

namespace mg {

template <typename t>
bool Allocate(t** Buf, i64 Size) {
  return (*Buf = (t*)malloc(Size * sizeof(t))) != nullptr;
}

template <typename t>
void Deallocate(t** Buf) {
  free(*Buf);
  *Buf = nullptr;
}

} // namespace mg
