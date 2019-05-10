#pragma once

#include "mg_assert.h"

namespace mg {

mg_T2i(t, u) expected<t, u>::
expected() : Ok(false) {}

mg_T2i(t, u) expected<t, u>::
expected(const t& ValIn) : Val(ValIn), Ok(true) {}

mg_T2i(t, u) expected<t, u>::
expected(const error<u>& ErrIn) : Err(ErrIn), Ok(false) {}

mg_T2i(t, u) t& expected<t, u>::
operator*() { return Val; }

/* (Safely) get the value with the added check */
mg_T2i(t, u) t&
Value(expected<t, u>& E) { mg_Assert(E.Ok); return E.Val; }

mg_T2i(t, u) t* expected<t, u>::
operator->() { return &**this; }

/* Get the error */
mg_T2i(t, u) error<u>&
Error(expected<t, u>& E) { mg_Assert(!E.Ok); return E.Err; }

/* Bool operator */
mg_T2i(t, u) expected<t, u>::
operator bool() const { return Ok; }

} // namespace mg

