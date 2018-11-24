/* Generic algorithms to replace <algorithm> */

#pragma once
namespace mg {

template <typename t>
t Min(const t& a, const t& b);

template <typename i, typename t>
i Find(i Begin, i End, const t& Val);

template <typename t1, typename t2>
bool Contains(t1 Big, t2 Small);

template <typename i, typename t>
i FindIf(i Begin, i End, const t& Func);

} // namespace mg

#include "mg_algorithm.inl"
