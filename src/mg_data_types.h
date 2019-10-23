// TODO: add RGB types

#pragma once

#include "mg_common.h"
#include "mg_enum.h"
#include "mg_macros.h"

mg_Enum(dtype, int,
  int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64)

/*
Dispatch some code depending on Type. To use, define a Body macro which
contains the code to run. Presumably the code makes use of Type. */
#define mg_DispatchOnType(Type)
#define mg_DispatchOnInt(Type)
#define mg_DispatchOnFloat(Type)
#define mg_DispatchOn2Types(Type1, Type2)
#define mg_DispatchOnFloat1(Type1, Type2) // Type1 is floating point
#define mg_DispatchOnFloat2(Type1, Type2) // Type2 is floating point

namespace mg {

mg_T(t) struct dtype_traits          { static inline const dtype Type = dtype::__Invalid__; };
template <> struct dtype_traits<i8 > { static inline const dtype Type = dtype::   int8    ; };
template <> struct dtype_traits<u8 > { static inline const dtype Type = dtype::  uint8    ; };
template <> struct dtype_traits<i16> { static inline const dtype Type = dtype::  int16    ; };
template <> struct dtype_traits<u16> { static inline const dtype Type = dtype:: uint16    ; };
template <> struct dtype_traits<i32> { static inline const dtype Type = dtype::  int32    ; };
template <> struct dtype_traits<u32> { static inline const dtype Type = dtype:: uint32    ; };
template <> struct dtype_traits<i64> { static inline const dtype Type = dtype::  int64    ; };
template <> struct dtype_traits<u64> { static inline const dtype Type = dtype:: uint64    ; };
template <> struct dtype_traits<f32> { static inline const dtype Type = dtype::float32    ; };
template <> struct dtype_traits<f64> { static inline const dtype Type = dtype::float64    ; };

mg_T(t)
bool  ISameType       (dtype Type);
bool  IsIntegral      (dtype Type);
bool  IsSigned        (dtype Type);
bool  IsUnsigned      (dtype Type);
bool  IsFloatingPoint (dtype Type);
int   SizeOf          (dtype Type);
int   BitSizeOf       (dtype Type);
dtype IntType         (dtype Type);
dtype FloatType       (dtype Type);
dtype UnsignedType    (dtype Type);
dtype SignedType      (dtype Type);

} // namespace mg

#include "mg_data_types.inl"
