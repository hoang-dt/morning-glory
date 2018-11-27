/* Generic algorithms to replace <algorithm> */

#pragma once
namespace mg {

template <typename t>
t Min(t a, t b);

template <typename i, typename t>
i Find(i Begin, i End, const t& Val);

template <typename i, typename t>
i FindLast(i ReverseBegin, i ReverseEnd, const t& Val);

template <typename t1, typename t2>
bool Contains(const t1& Big, const t2& Small);

template <typename i, typename t>
i FindIf(i Begin, i End, const t& Func);

} // namespace mg

#include "mg_algorithm.inl"
