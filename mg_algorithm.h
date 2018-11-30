/* Generic algorithms to replace <algorithm> */

#pragma once
namespace mg {

template <typename t>
t Min(t a, t b);
template <typename t>
t Max(t a, t b);

template <typename i, typename f>
i MaxElement(i Begin, i End, const f& CompareFunc);

template <typename i>
struct MinMax { i Min, Max; };
template <typename i>
MinMax<i> MinMaxElement(i Begin, i End);

template <typename i, typename t>
i Find(i Begin, i End, const t& Val);

template <typename i, typename t>
i FindLast(i ReverseBegin, i ReverseEnd, const t& Val);

template <typename t1, typename t2>
bool Contains(const t1& Big, const t2& Small);

template <typename i, typename t>
i FindIf(i Begin, i End, const t& Func);

template <typename t>
void Swap(t* A, t* B);

} // namespace mg

#include "mg_algorithm.inl"
