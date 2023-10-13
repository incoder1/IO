#ifndef __IO_TEXT_API__DETAILCYRYLIC_MODEL_HPP_INCLUDED__
#define __IO_TEXT_API__DETAILCYRYLIC_MODEL_HPP_INCLUDED__


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
koi8r_sequence_model() noexcept;

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
win1251_sequence_model() noexcept;

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
iso_8859_5_sequence_model() noexcept;

} // namespace io

} // namespace detail

#endif // __IO_TEXT_API__DETAILCYRYLIC_MODEL_HPP_INCLUDED__
