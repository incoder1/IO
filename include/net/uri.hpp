#ifndef __URI_HPP_INCLUDED__
#define __URI_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <functional>
#include <ostream>
#include <conststring.hpp>

namespace io {

namespace net {


class uri;
DECLARE_IPTR(uri);

class IO_PUBLIC_SYMBOL uri:public object {
public:

	static s_uri parse(std::error_code& ec, const char* str) noexcept;

	static uint16_t IO_NO_INLINE default_port_for_scheme(const char* scheme) noexcept;

private:


	friend class nobadalloc<uri>;

	uri(
	    const_string&& scheme,
	    uint16_t port,
	    const_string&& host,
	    const_string&& user_info,
	    const_string&& path,
	    const_string&& query,
	    const_string&& fragment) noexcept;
public:

	inline const const_string& scheme() const noexcept
	{
		return scheme_;
	}

	uint16_t port() const noexcept
	{
		return port_;
	}

	inline const const_string& host() const noexcept
	{
		return host_;
	}

	inline const const_string& user_info() const noexcept
	{
		return user_info_;
	}

	inline const const_string& path() const noexcept
	{
		return path_;
	}

	inline const const_string& query() const noexcept
	{
		return query_;
	}

	inline const const_string& fragment() const noexcept
	{
		return fragment_;
	}
	std::size_t hash() const noexcept;
private:
	uint16_t port_;
	const_string scheme_;
	const_string host_;
	const_string user_info_;
	const_string path_;
	const_string query_;
	const_string fragment_;
};

std::ostream& IO_PUBLIC_SYMBOL operator<<(std::ostream& ret, const uri& uri);

} // namespace net

} // namespace io

#endif // __URI_HPP_INCLUDED__
