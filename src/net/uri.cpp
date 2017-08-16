#include "../stdafx.hpp"
#include "net/uri.hpp"

namespace io {

namespace net {

// uri


uri uri::parse(std::error_code& ec, const char* str) noexcept
{
	return uri(protocol::http, 80, const_string(), const_string(), const_string(), const_string());
}

uri::uri(protocol scheme,uint16_t port,
		const_string&& host,const_string&& absolute,
		const_string&& relative, const_string&& fragment) noexcept:
	port_(port),
	scheme_(scheme),
	host_( std::forward<const_string>(host) ),
	absolute_( std::forward<const_string>(absolute) ),
	relative_( std::forward<const_string>(relative) ),
	fragment_( std::forward<const_string>(fragment_) )
{}

}  // namespace io

} // namespace net
