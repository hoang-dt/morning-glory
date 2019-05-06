#pragma once

#include "mg_common.h"
#include "mg_error.h"
#include "mg_macros.h"

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
  expected(const t& ValIn);
  expected(const error<u>& ErrIn);

  /* Get the value through the pointer syntax */
  t& operator*();
  /* Mimic pointer semantics */
  t* operator->();
  /* Bool operator */
  explicit operator bool() const;
}; // struct expected

mg_T2(t, u) t& Value(expected<t, u>& E);
mg_T2(t, u) error<u>& Error(expected<t, u>& E);

} // namespace mg

#include "mg_expected.inl"

