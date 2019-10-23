/* Generic algorithms to replace <algorithm> */
#pragma once

#include "mg_macros.h"

namespace mg {

mg_T(t) t Min(const t& a, const t& b);
mg_T(t) t Max(const t& a, const t& b);

mg_T(i) i MaxElem(i Beg, i End);
mg_TT(i, f) i MaxElem(i Beg, i End, const f& Comp);
mg_T(i) struct min_max { i Min, Max; };
mg_T(i) min_max<i> MinMaxElem(i Beg, i End);
mg_TT(i, f) min_max<i> MinMaxElem(i Beg, i End, const f& Comp);

mg_TT(i, t) i Find(i Beg, i End, const t& Val);
mg_TT(i, t) i FindLast(i RevBeg, i RevEnd, const t& Val);
mg_TT(t1, t2) bool Contains(const t1& Collection, const t2& Elem);
mg_TT(i, f) i FindIf(i Beg, i End, const f& Pred);

mg_T(t) void Swap(t* A, t* B);

mg_TT(i, t) void Fill(i Beg, i End, const t& Val);

/* Only work with random access iterator */
mg_T(i) void Reverse(i Beg, i End);

mg_T(i) int FwdDist(i Beg, i End);

} // namespace mg

#include "mg_algorithm.inl"
