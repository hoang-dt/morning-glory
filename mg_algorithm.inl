#pragma once

#include "mg_macros.h"

namespace mg {

template <typename t> mg_ForceInline
t Min(t a, t b) {
  return b < a ? b : a;
}
template <typename t> mg_ForceInline
t Max(t a, t b) {
  return a < b ? b : a;
}

template <typename i, typename f>
i MaxElement(i Begin, i End, const f& CompareFunc) {
  auto MaxElem = Begin;
  for (i Pos = Begin; Pos != End; ++Pos) {
    if (CompareFunc(*MaxElem, *Pos))
      MaxElem = Pos;
  }
  return MaxElem;
}

template <typename i>
MinMax<i> MinMaxElement(i Begin, i End) {
  auto MinElem = Begin;
  auto MaxElem = Begin;
  for (i Pos = Begin; Pos != End; ++Pos) {
    if (*Pos < *MinElem)
      MinElem = Pos;
    else if (*Pos > *MaxElem)
      MaxElem = Pos;
  }
  return MinMax<i>{ MinElem, MaxElem };
}

template <typename i, typename f>
MinMax<i> MinMaxElement(i Begin, i End, const f& CompareFunc) {
  auto MinElem = Begin;
  auto MaxElem = Begin;
  for (i Pos = Begin; Pos != End; ++Pos) {
    if (CompareFunc(*Pos, *MinElem))
      MinElem = Pos;
    else if (CompareFunc(*MaxElem, *Pos))
      MaxElem = Pos;
  }
  return MinMax<i>{ MinElem, MaxElem };
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

template <typename t>
void Swap(t* A, t* mg_Restrict B) {
  t T = *A;
  *A = *B;
  *B = T;
}

template <typename i, typename t>
void Fill(i Begin, i End, const t& Val) {
for (i Iter = Begin; Iter != End; ++Iter)
  *Iter = Val;
}

template <typename i>
void Reverse(i Begin, i End) {
  auto Iter1 = Begin;
  auto Iter2 = End - 1;
  while (Iter1 < Iter2) {
    Swap(Iter1, Iter2);
    ++Iter1;
    --Iter2;
  }
}

} // namespace mg
