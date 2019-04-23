#pragma once

#include "mg_types.h"

namespace mg {

template <typename t>
bool MatchTypes(data_type Type) {
  if (Type == mg::data_type::float64) {
    return IsSameType<t, f64>::Result;
  } else if (Type == mg::data_type::float32) {
    return IsSameType<t, f32>::Result;
  } else if (Type == mg::data_type::int64) {
    return IsSameType<t, i64>::Result;
  } else if (Type == mg::data_type::int32) {
    return IsSameType<t, i32>::Result;
  } else if (Type == mg::data_type::int16) {
    return IsSameType<t, i16>::Result;
  } else if (Type == mg::data_type::int8) {
    return IsSameType<t, i8>::Result;
  } else {
    mg_Assert(false, "type not supported");
  }
  return false;
}

} //namespace mg
