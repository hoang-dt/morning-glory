#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

namespace mg {

/* Something to replace std::array */
#define TemplateArr template <typename t, int N>
TemplateArr
struct array {
  t Arr[N];
  t& operator[](int Idx) { assert(Idx < N); return Arr[Idx]; }
};
TemplateArr t* Begin(array<t, N>& A) { return &A.Arr[0]; }
TemplateArr t* End(array<t, N>& A) { return &A.Arr[0] + N; }
TemplateArr int Size(array<t, N>&) { return N; }
#undef TemplateArr

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
template <typename t>
struct v2 {
  union {
    struct { t X, Y; };
    struct { t U, V; };
    t E[2];
  };
  t& operator[](int Idx) { assert(Idx < 2); return E[Idx]; }
};
using v2i  = v2<i32>;
using v2u  = v2<u32>;
using v2l  = v2<i64>;
using v2ul = v2<u64>;
using v2f  = v2<f32>;
using v2d  = v2<f64>;

template <typename t>
struct v3 {
  union {
    struct { t X, Y, Z; };
    struct { t U, V, __; };
    struct { t R, G, B; };
    struct { v2<t> XY; t Ignored0_; };
    struct { t Ignored1_; v2<t> YZ; };
    struct { v2<t> UV; t Ignored2_; };
    struct { t Ignored3_; v2<t> V__; };
    t E[3];
  };
  t& operator[](int Idx) { assert(Idx < 3); return E[Idx]; }
};
using v3i  = v3<i32>;
using v3u  = v3<u32>;
using v3l  = v3<i64>;
using v3ul = v3<u64>;
using v3f  = v3<f32>;
using v3d  = v3<f64>;
#pragma GCC diagnostic pop

} // namespace mg
