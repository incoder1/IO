#ifndef __IO_TLS_ERROR_HPP__
#define __IO_TLS_ERROR_HPP__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>

#include <gnutls/gnutls.h>

namespace io {

namespace net {

namespace tls {

class IO_PUBLIC_SYMBOL error_category final: public std::error_category
{
public:
	constexpr error_category() noexcept:
		std::error_category()
	{}
	virtual const char* name() const noexcept override;
	virtual std::string message( int condition ) const override;
}; // error_category

std::error_code IO_PUBLIC_SYMBOL make_error_code(int tls_err) noexcept;

} // namespace tls

} // namespace net

} // namespace io


#endif // __IO_TLS_ERROR_HPP__
