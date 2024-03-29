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
  } else if (Type == mg::dtype::uint64) {\
    Body(u64)\
  } else if (Type == mg::dtype::int32) {\
    Body(i32)\
  } else if (Type == mg::dtype::uint32) {\
    Body(u32)\
  } else if (Type == mg::dtype::int16) {\
    Body(i16)\
  } else if (Type == mg::dtype::uint16) {\
    Body(u16)\
  } else if (Type == mg::dtype::int8) {\
    Body(i8)\
  } else if (Type == mg::dtype::uint8) {\
    Body(u8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

/* Type1 is fixed, switch based on Type2 */
#define mg_DispatchOn2TypesHelper1(Type1, Type2)\
  if (Type2 == mg::dtype::float64) {\
    Body(Type1, f64)\
  } else if (Type2 == mg::dtype::float32) {\
    Body(Type1, f32)\
  } else if (Type2 == mg::dtype::int64) {\
    Body(Type1, i64)\
  } else if (Type2 == mg::dtype::uint64) {\
    Body(Type1, u64)\
  } else if (Type2 == mg::dtype::int32) {\
    Body(Type1, i32)\
  } else if (Type2 == mg::dtype::uint32) {\
    Body(Type1, u32)\
  } else if (Type2 == mg::dtype::int16) {\
    Body(Type1, i16)\
  } else if (Type2 == mg::dtype::uint16) {\
    Body(Type1, u16)\
  } else if (Type2 == mg::dtype::int8) {\
    Body(Type1, i8)\
  } else if (Type2 == mg::dtype::uint8) {\
    Body(Type1, u8)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

/* Type2 is fixed, switch based on Type1 */
#define mg_DispatchOn2TypesHelper2(Type1, Type2)\
  if (Type1 == mg::dtype::float64) {\
    Body(f64, Type2)\
  } else if (Type1 == mg::dtype::float32) {\
    Body(f32, Type2)\
  } else if (Type1 == mg::dtype::int64) {\
    Body(i64, Type2)\
  } else if (Type1 == mg::dtype::uint64) {\
    Body(u64, Type2)\
  } else if (Type1 == mg::dtype::int32) {\
    Body(i32, Type2)\
  } else if (Type1 == mg::dtype::uint32) {\
    Body(u32, Type2)\
  } else if (Type1 == mg::dtype::int16) {\
    Body(i16, Type2)\
  } else if (Type1 == mg::dtype::uint16) {\
    Body(u16, Type2)\
  } else if (Type1 == mg::dtype::int8) {\
    Body(i8, Type2)\
  } else if (Type1 == mg::dtype::uint8) {\
    Body(u8, Type2)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#undef mg_DispatchOn2Types
#define mg_DispatchOn2Types(Type1, Type2)\
  if (Type1 == mg::dtype::float64) {\
    mg_DispatchOn2TypesHelper1(f64, Type2)\
  } else if (Type1 == mg::dtype::float32) {\
    mg_DispatchOn2TypesHelper1(f32, Type2)\
  } else if (Type1 == mg::dtype::int64) {\
    mg_DispatchOn2TypesHelper1(i64, Type2)\
  } else if (Type1 == mg::dtype::uint64) {\
    mg_DispatchOn2TypesHelper1(u64, Type2)\
  } else if (Type1 == mg::dtype::int32) {\
    mg_DispatchOn2TypesHelper1(i32, Type2)\
  } else if (Type1 == mg::dtype::uint32) {\
    mg_DispatchOn2TypesHelper1(u32, Type2)\
  } else if (Type1 == mg::dtype::int16) {\
    mg_DispatchOn2TypesHelper1(i16, Type2)\
  } else if (Type1 == mg::dtype::uint16) {\
    mg_DispatchOn2TypesHelper1(u16, Type2)\
  } else if (Type1 == mg::dtype::int8) {\
    mg_DispatchOn2TypesHelper1(i8, Type2)\
  } else if (Type1 == mg::dtype::uint8) {\
    mg_DispatchOn2TypesHelper1(u8, Type2)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#undef mg_DispatchOnFloat1
#define mg_DispatchOnFloat1(Type1, Type2)\
  if (Type1 == mg::dtype::float64) {\
    mg_DispatchOn2TypesHelper1(f64, Type2)\
  } else if (Type1 == mg::dtype::float32) {\
    mg_DispatchOn2TypesHelper1(f32, Type2)\
  } else {\
    mg_Assert(false, "type not supported");\
  }

#undef mg_DispatchOnFloat2
#define mg_DispatchOnFloat2(Type1, Type2)\
  if (Type2 == mg::dtype::float64) {\
    mg_DispatchOn2TypesHelper2(Type1, f64)\
  } else if (Type1 == mg::dtype::float32) {\
    mg_DispatchOn2TypesHelper2(Type1, f32)\
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

mg_Inline bool
IsIntegral(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  :
    case dtype::int16  :
    case dtype::uint16 :
    case dtype::int32  :
    case dtype::uint32 :
    case dtype::int64  :
    case dtype::uint64 : return true;
    case dtype::float32:
    case dtype::float64: return false;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

mg_Inline bool
IsSigned(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::int16  :
    case dtype::int32  :
    case dtype::int64  : return true;
    case dtype::uint8  :
    case dtype::uint16 :
    case dtype::uint32 :
    case dtype::uint64 : return false;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

mg_Inline bool
IsUnsigned(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::int16  :
    case dtype::int32  :
    case dtype::int64  : return false;
    case dtype::uint8  :
    case dtype::uint16 :
    case dtype::uint32 :
    case dtype::uint64 : return true;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

mg_Inline bool
IsFloatingPoint(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  :
    case dtype::int16  :
    case dtype::uint16 :
    case dtype::int32  :
    case dtype::uint32 :
    case dtype::int64  :
    case dtype::uint64 : return false;
    case dtype::float32:
    case dtype::float64: return true;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

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
ISameType(dtype Type) {
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
    case dtype::float32: return dtype::int32;
    case dtype::float64: return dtype::int64;
    default: mg_Assert(false, "type unsupported");
  };
  return dtype(dtype::__Invalid__);
}

mg_Inline dtype
FloatType(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  :
    case dtype::int16  :
    case dtype::uint16 :
    case dtype::int32  :
    case dtype::uint32 :
    case dtype::float32: return dtype::float32;
    case dtype::int64  :
    case dtype::uint64 :
    case dtype::float64: return dtype::float64;
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

mg_Inline dtype
SignedType(dtype Type) {
  switch (Type) {
    case dtype::int8   :
    case dtype::uint8  : return dtype::int8;
    case dtype::int16  :
    case dtype::uint16 : return dtype::int16;
    case dtype::int32  :
    case dtype::uint32 : return dtype::int32;
    case dtype::int64  :
    case dtype::uint64 : return dtype::int64;
    default: mg_Assert(false, "type unsupported");
  };
  return dtype(dtype::__Invalid__);
}

} //namespace mg
