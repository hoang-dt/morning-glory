#pragma once

namespace mg {

mg_Ti(t) t Min(const t& a, const t& b) { return b < a ? b : a; }
mg_Ti(t) t Max(const t& a, const t& b) { return a < b ? b : a; }

mg_T(i) i
MaxElem(i Beg, i End) {
  auto MaxElem = Beg;
  for (i Pos = Beg; Pos != End; ++Pos) {
    if (*MaxElem < *Pos)
      MaxElem = Pos;
  }
  return MaxElem;
}

mg_TT(i, f) i
MaxElem(i Beg, i End, f& Comp) {
  auto MaxElem = Beg;
  for (i Pos = Beg; Pos != End; ++Pos) {
    if (Comp(*MaxElem, *Pos))
      MaxElem = Pos;
  }
  return MaxElem;
}

mg_T(i) min_max<i>
MinMaxElem(i Beg, i End) {
  auto MinElem = Beg;
  auto MaxElem = Beg;
  for (i Pos = Beg; Pos != End; ++Pos) {
    if (*Pos < *MinElem)
      MinElem = Pos;
    else if (*Pos > *MaxElem)
      MaxElem = Pos;
  }
  return min_max<i>{MinElem, MaxElem};
}

mg_TT(i, f) min_max<i>
MinMaxElem(i Beg, i End, const f& Comp) {
  auto MinElem = Beg;
  auto MaxElem = Beg;
  for (i Pos = Beg; Pos != End; ++Pos) {
    if (Comp(*Pos, *MinElem))
      MinElem = Pos;
    else if (Comp(*MaxElem, *Pos))
      MaxElem = Pos;
  }
  return min_max<i>{ MinElem, MaxElem };
}

mg_TT(i, t) i
Find(i Beg, i End, const t& Val) {
  for (i Pos = Beg; Pos != End; ++Pos) {
    if (*Pos == Val)
      return Pos;
  }
  return End;
}

mg_TT(i, t) i
FindLast(i RevBeg, i RevEnd, const t& Val) {
  for (i Pos = RevBeg; Pos != RevEnd; --Pos) {
    if (*Pos == Val)
      return Pos;
  }
  return RevEnd;
}

mg_TTi(t1, t2) bool
Contains(const t1& Collection, const t2& Elem) {
  return Find(Begin(Collection), End(Collection), Elem) != End(Collection);
}

mg_TT(i, f) i
FindIf(i Beg, i End, const f& Pred) {
  for (i Pos = Beg; Pos != End; ++Pos) {
    if (Pred(*Pos))
      return Pos;
  }
  return End;
}

mg_Ti(t) constexpr void
Swap(t* A, t* mg_Restrict B) {
  t T = *A;
  *A = *B;
  *B = T;
}

mg_Ti(it) constexpr void
IterSwap(it A, it B) {
  Swap(&(*A), &(*B));
}

mg_TT(i, t) void
Fill(i Beg, i End, const t& Val) {
  for (i It = Beg; It != End; ++It)
    *It = Val;
}

mg_T(i) void
Reverse(i Beg, i End) {
  auto It1 = Beg;
  auto It2 = End - 1;
  while (It1 < It2) {
    Swap(It1, It2);
    ++It1;
    --It2;
  }
}

mg_T(i) int
FwdDist(i Beg, i End) {
  int Dist = 0;
  while (Beg != End) {
    ++Dist;
    ++Beg;
  }
  return Dist;
}

} // namespace mg
