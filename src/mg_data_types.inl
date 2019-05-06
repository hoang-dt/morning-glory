#pragma once

#include "mg_assert.h"

#undef mg_DispatchOnType
#define mg_DispatchOnType(Type)\
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

#undef mg_DispatchOnInt
#define mg_DispatchOnInt(Type)\
  if (Type == mg::data_type::int64) {\
    Body(i64)\
  } else if (Type == mg::data_type::int32) {\
    Body(i32)\
  } else if (Type == mg::data_type::int16) {\
    Body(i16)\
  } else if (Type == mg::data_type::int8) {\
    Body(i8)\
    mg_Assert(false, "type not supported");\
  }

#undef mg_DispatchOnFloat
#define mg_DispatchOnFloat(Type)\
  if (Type == mg::data_type::float64) {\
    Body(f64)\
  } else if (Type == mg::data_type::float32) {\
    Body(f32)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

namespace mg {

mg_Inline int
SizeOf(data_type Type) {
  switch (Type) {
    case data_type::int8   : return 1;
    case data_type::uint8  : return 1;
    case data_type::int16  : return 2;
    case data_type::uint16 : return 2;
    case data_type::int32  : return 4;
    case data_type::uint32 : return 4;
    case data_type::int64  : return 8;
    case data_type::uint64 : return 8;
    case data_type::float32: return 4;
    case data_type::float64: return 8;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

mg_Inline int
BitSizeOf(data_type Type) { return 8 * SizeOf(Type); }

mg_T(t) bool
MatchTypes(data_type Type) {
  switch (Type) {
    case data_type::int8   : return is_same_type<t,  i8>::Value;
    case data_type::uint8  : return is_same_type<t,  u8>::Value;
    case data_type::int16  : return is_same_type<t, i16>::Value;
    case data_type::uint16 : return is_same_type<t, u16>::Value;
    case data_type::int32  : return is_same_type<t, i32>::Value;
    case data_type::uint32 : return is_same_type<t, u32>::Value;
    case data_type::int64  : return is_same_type<t, i64>::Value;
    case data_type::uint64 : return is_same_type<t, u64>::Value;
    case data_type::float32: return is_same_type<t, f32>::Value;
    case data_type::float64: return is_same_type<t, f64>::Value;
    default: mg_Assert(false, "type unsupported");
  };
  return false;
}

mg_Inline data_type
IntType(data_type Type) {
  switch (Type) {
    case data_type::int8   : return Type;
    case data_type::uint8  : return Type;
    case data_type::int16  : return Type;
    case data_type::uint16 : return Type;
    case data_type::int32  : return Type;
    case data_type::uint32 : return Type;
    case data_type::int64  : return Type;
    case data_type::uint64 : return Type;
    case data_type::float32: return data_type(data_type::int32);
    case data_type::float64: return data_type(data_type::int64);
    default: mg_Assert(false, "type unsupported");
  };
  return data_type(data_type::__Invalid__);
}

} //namespace mg
