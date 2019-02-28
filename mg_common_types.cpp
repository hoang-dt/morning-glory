#include "mg_assert.h"
#include "mg_common_types.h"

namespace mg {

int SizeOf(data_type Type) {
  switch (Type.Value) {
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

data_type IntType(data_type Type) {
  switch (Type.Value) {
    case data_type::int8   : return Type;
    case data_type::uint8  : return Type;
    case data_type::int16  : return Type;
    case data_type::uint16 : return Type;
    case data_type::int32  : return Type;
    case data_type::uint32 : return Type;
    case data_type::int64  : return Type;
    case data_type::uint64 : return Type;
    case data_type::float32: return data_type::int32;
    case data_type::float64: return data_type::int64;
    default: mg_Assert(false, "type unsupported");
  };
  return 0;
}

} // namespace mg
