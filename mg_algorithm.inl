#pragma once

namespace mg {

template <typename t>
t Min(const t& a, const t& b) {
  return a < b ? a : b;
}

template <typename i, typename t>
i Find(i Begin, i End, const t& Val) {
  for (i Pos = Begin; Pos != End; ++Pos) {
    if (*Pos == Val)
      return Pos;
  }
  return End;
}

template <typename i, typename t>
i FindLast(i ReverseBegin, i ReverseEnd, const t& Val) {
  for (i Pos = ReverseBegin; Pos != ReverseEnd; --Pos) {
    if (*Pos == Val)
      return Pos;
  }
  return ReverseEnd;
}

template <typename t1, typename t2>
bool Contains(const t1& Big, const t2& Small) {
  return Find(ConstBegin(Big), ConstEnd(Big), Small) != ConstEnd(Big);
}

template <typename i, typename t>
i FindIf(i Begin, i End, const t& Func) {
  for (i Pos = Begin; Pos != End; ++Pos) {
    if (Func(*Pos))
      return Pos;
  }
  return End;
}

} // namespace mg
