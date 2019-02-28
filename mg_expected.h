#pragma once

#include "mg_error.h"

namespace mg {

/* Store either a value or an error */
template <typename t>
struct expected {
  union {
    t Val;
    error Err;
  };
  bool Ok = false;
  expected();
  expected(const t& Val);
  expected(error Err);

  /* Get the value through the pointer syntax */
  t& operator*();
  const t& operator*() const;
  /* Mimic pointer semantics */
  t* operator->();
  const t* operator->() const;
  /* Bool operator */
  explicit operator bool() const;
}; // struct expected

template <typename t> t& Value(expected<t>& E);
template <typename t> const t& Value(const expected<t>& E);

}// namespace mg

#include "mg_expected.inl"

