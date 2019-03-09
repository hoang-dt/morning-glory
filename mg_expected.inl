#pragma once

#include "mg_assert.h"
#include "mg_error.h"
#include "mg_macros.h"

namespace mg {

template <typename t, typename u> mg_ForceInline
expected<t, u>::expected() : Ok(false) {}
template <typename t, typename u> mg_ForceInline
expected<t, u>::expected(const t& Val) : Val(Val), Ok(true) {}
template <typename t, typename u> mg_ForceInline
expected<t, u>::expected(error<u> Err) : Err(Err), Ok(false) {}

template <typename t, typename u> mg_ForceInline
t& expected<t, u>::operator*() { return Val; }
template <typename t, typename u> mg_ForceInline
const t& expected<t, u>::operator*() const { return Val; }

/* (Safely) get the value with the added check */
template <typename t, typename u> mg_ForceInline
t& GetValue(expected<t, u>& E) { mg_Assert(E.Ok); return E.Val; }
template <typename t, typename u> mg_ForceInline
const t& GetValue(const expected<t, u>& E) { mg_Assert(E.Ok); return E.Val; }

template <typename t, typename u> mg_ForceInline
t* expected<t, u>::operator->() { return &**this; }
template <typename t, typename u> mg_ForceInline
const t* expected<t, u>::operator->() const { return &**this; }

/* Get the error */
template <typename t, typename u> mg_ForceInline
error<u>& GetError(expected<t, u>& E) { mg_Assert(!E.Ok); return E.Err; }
template <typename t, typename u> mg_ForceInline
const error<u>& GetError(const expected<t, u>& E) { mg_Assert(!E.Ok); return E.Err; }

/* Bool operator */
template <typename t, typename u> mg_ForceInline
expected<t, u>::operator bool() const { return Ok; }

} // namespace mg

