#pragma once

#include "mg_error.h"

namespace mg {

/* Store either a value or an error */
template <typename t, typename u = err_code>
struct expected {
  union {
    t Val;
    error<u> Err;
  };
  bool Ok = false;
  expected();
  expected(const t& Val);
  expected(error<u> Err);

  /* Get the value through the pointer syntax */
  t& operator*();
  const t& operator*() const;
  /* Mimic pointer semantics */
  t* operator->();
  const t* operator->() const;
  /* Bool operator */
  explicit operator bool() const;
}; // struct expected

template <typename t, typename u> t& Value(expected<t, u>& E);
template <typename t, typename u> const t& Value(const expected<t, u>& E);

}// namespace mg

#include "mg_expected.inl"

