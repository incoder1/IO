/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_TEXT_API_DETAIL_GREEK_MODEL_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_GREEK_MODEL_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "single_byte_prober.hpp"

namespace io {

namespace detail {

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
win1253_sequence_model() noexcept;

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
iso_8859_7_sequence_model() noexcept;

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_GREEK_MODEL_HPP_INCLUDED__
