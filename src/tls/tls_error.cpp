#include "stdafx.hpp"
#include "tls_error.hpp"

namespace io {

namespace net {

namespace tls {

// error_category
const char* error_category::name() const noexcept
{
	return "Transport Layer Security";
}

std::string error_category::message( int condition ) const
{
	return ::gnutls_strerror(condition);
}

// free functions
std::error_code IO_PUBLIC_SYMBOL make_error_code(int tls_err) noexcept
{
	static error_category _tls_cat;
	return std::error_code( tls_err, _tls_cat);
}

} // namespace tls

} // namespace net

} // namespace io
