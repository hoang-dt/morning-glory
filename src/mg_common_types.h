#pragma once

#include "mg_enum.h"

mg_Enum(data_type, int,
  int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64)

#define TypeChooser(Type)\
  if (Type == mg::data_type::float64) {\
    Body(f64)\
  } else if (Type == mg::data_type::float32) {\
    Body(f32)\
  } else if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else if (Type == mg::data_type::int16) {\
    Body(i16)\
  } else if (Type == mg::data_type::int8) {\
    Body(i8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#define TypeChooserInt(Type)\
  if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else if (Type == mg::data_type::int16) {\
    Body(i16)\
  } else if (Type == mg::data_type::int8) {\
    Body(i8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#define TypeChooserInt32And64(Type)\
  if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#define TypeChooserFloat(Type)\
  if (Type == mg::data_type::float64) {\
    Body(f64)\
  } else if (Type == mg::data_type::float32) {\
    Body(f32)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

namespace mg {

int SizeOf(data_type Type);
int BitSizeOf(data_type Type);
data_type IntType(data_type InputType);
template <typename t>
bool MatchTypes(data_type Type);

} // namespace mg

#include "mg_common_types.inl"
