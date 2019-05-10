/* Generic algorithms to replace <algorithm> */
#pragma once

#include "mg_macros.h"

namespace mg {

mg_T(t) t Min(const t& a, const t& b);
mg_T(t) t Max(const t& a, const t& b);

mg_T2(i, f) i MaxElem(i Beg, i End, const f& Comp);
mg_T(i) struct MinMax { i Min, Max; };
mg_T(i) MinMax<i> MinMaxElem(i Beg, i End);
mg_T2(i, f) MinMax<i> MinMaxElem(i Beg, i End, const f& Comp);

mg_T2(i, t) i Find(i Beg, i End, const t& Val);
mg_T2(i, t) i FindLast(i RevBeg, i RevEnd, const t& Val);
mg_T2(t1, t2) bool Contains(const t1& Collection, const t2& Elem);
mg_T2(i, f) i FindIf(i Beg, i End, const f& Pred);

mg_T(t) void Swap(t* A, t* B);

mg_T2(i, t) void Fill(i Beg, i End, const t& Val);

/* Only work with random access iterator */
mg_T(i) void Reverse(i Beg, i End);

mg_T(i) int FwdDist(i Beg, i End);

} // namespace mg

#include "mg_algorithm.inl"
