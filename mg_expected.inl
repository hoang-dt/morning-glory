#pragma once

#include "mg_assert.h"
#include "mg_error.h"
#include "mg_macros.h"

namespace mg {


template <typename t> mg_ForceInline
expected<t>::expected() : Ok(false) {}
template <typename t> mg_ForceInline
expected<t>::expected(const t& Val) : Val(Val), Ok(true) {}
template <typename t> mg_ForceInline
expected<t>::expected(error Err) : Err(Err), Ok(false) {}

template <typename t> mg_ForceInline
t& expected<t>::operator*() { return Val; }
template <typename t> mg_ForceInline
const t& expected<t>::operator*() const { return Val; }

/* (Safely) get the value with the added check */
template <typename t> mg_ForceInline
t& GetValue(expected<t>& E) { mg_Assert(E.Ok); return E.Val; }
template <typename t> mg_ForceInline
const t& GetValue(const expected<t>& E) { mg_Assert(E.Ok); return E.Val; }

template <typename t> mg_ForceInline
t* expected<t>::operator->() { return &**this; }
template <typename t> mg_ForceInline
const t* expected<t>::operator->() const { return &**this; }

/* Get the error */
template <typename t> mg_ForceInline
error& GetError(expected<t>& E) { mg_Assert(!E.Ok); return E.Err; }
template <typename t> mg_ForceInline
const error& GetError(const expected<t>& E) { mg_Assert(!E.Ok); return E.Err; }

/* Bool operator */
template <typename t> mg_ForceInline
expected<t>::operator bool() const { return Ok; }

} // namespace mg

