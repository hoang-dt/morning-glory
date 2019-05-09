#pragma once

#include "mg_assert.h"

#undef mg_DispatchOnType
#define mg_DispatchOnType(Type)\
  if (Type == mg::dtype::float64) {\
    Body(f64)\
  } else if (Type == mg::dtype::float32) {\
    Body(f32)\
  } else if (Type == mg::dtype::int64) {\
    Body(i64)\
  } else if (Type == mg::dtype::int32) {\
    Body(i32)\
  } else if (Type == mg::dtype::int16) {\
    Body(i16)\
  } else if (Type == mg::dtype::int8) {\
    Body(i8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#undef mg_DispatchOnInt
#define mg_DispatchOnInt(Type)\
  if (Type == mg::dtype::int64) {\
    Body(i64)\
  } else if (Type == mg::dtype::int32) {\
    Body(i32)\
  } else if (Type == mg::dtype::int16) {\
    Body(i16)\
  } else if (Type == mg::dtype::int8) {\
    Body(i8)\
    mg_Assert(false, "type not supported");\
  }

#undef mg_DispatchOnFloat
#define mg_DispatchOnFloat(Type)\
  if (Type == mg::dtype::float64) {\
    Body(f64)\
  } else if (Type == mg::dtype::float32) {\
    Body(f32)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

namespace mg {

mg_Inline int
SizeOf(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  : return 1;
    case dtype::int16  :
    case dtype::uint16 : return 2;
    case dtype::int32  :
    case dtype::uint32 :
    case dtype::float32: return 4;
    case dtype::int64  :
    case dtype::uint64 :
    case dtype::float64: return 8;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

mg_Inline int
BitSizeOf(dtype Type) { return 8 * SizeOf(Type); }

mg_T(t) bool
MatchTypes(dtype Type) {
  switch (Type) {
    case dtype::int8   : return is_same_type<t,  i8>::Value;
    case dtype::uint8  : return is_same_type<t,  u8>::Value;
    case dtype::int16  : return is_same_type<t, i16>::Value;
    case dtype::uint16 : return is_same_type<t, u16>::Value;
    case dtype::int32  : return is_same_type<t, i32>::Value;
    case dtype::uint32 : return is_same_type<t, u32>::Value;
    case dtype::int64  : return is_same_type<t, i64>::Value;
    case dtype::uint64 : return is_same_type<t, u64>::Value;
    case dtype::float32: return is_same_type<t, f32>::Value;
    case dtype::float64: return is_same_type<t, f64>::Value;
    default: mg_Assert(false, "type unsupported");
  };
  return false;
}

mg_Inline dtype
IntType(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  :
    case dtype::int16  :
    case dtype::uint16 :
    case dtype::int32  :
    case dtype::uint32 :
    case dtype::int64  :
    case dtype::uint64 : return Type;
    case dtype::float32: return dtype(dtype::int32);
    case dtype::float64: return dtype(dtype::int64);
    default: mg_Assert(false, "type unsupported");
  };
  return dtype(dtype::__Invalid__);
}

mg_Inline dtype
UnsignedType(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  : return dtype::uint8;
    case dtype::int16  :
    case dtype::uint16 : return dtype::uint16;
    case dtype::int32  :
    case dtype::uint32 : return dtype::uint32;
    case dtype::int64  :
    case dtype::uint64 : return dtype::uint64;
    default: mg_Assert(false, "type unsupported");
  };
  return dtype(dtype::__Invalid__);
}

} //namespace mg
