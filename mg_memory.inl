#pragma once

#include <stdlib.h>
#include "mg_assert.h"
#include "mg_types.h"

namespace mg {

template <typename t>
bool Allocate(t** Ptr, i64 Size) {
  mg_Assert(!(*Ptr), "Pointer not freed before allocating new memory");
  return (*Ptr = (t*)malloc(Size * sizeof(t))) != nullptr;
}

template <typename t>
void Deallocate(t** Ptr) {
  free(*Ptr);
  *Ptr = nullptr;
}

} // namespace mg
