#ifndef __URI_HPP_INCLUDED__
#define __URI_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <functional>

#include <conststring.hpp>

namespace io {

namespace net {

enum class protocol
{
	http,
	https,
	ftp,
	file
};

inline uint16_t default_port(protocol prot) noexcept
{
	switch(prot) {
	case protocol::http:
		return 80;
	case protocol::https:
		return 443;
	case protocol::ftp:
		return 21;
	default:
		return static_cast<uint16_t>(-1);
	}
}

class IO_PUBLIC_SYMBOL uri {
public:

	static uri parse(std::error_code& ec, const char* str) noexcept;

	constexpr uri() noexcept:
		port_(0),
		scheme_(),
		host_(),
		user_info_(),
		path_(),
		query_(),
		fragment_()
	{}

	uri(
		const_string&& scheme,
		uint16_t port,
		const_string&& host,
		const_string&& user_info,
		const_string&& path,
		const_string&& query,
		const_string&& fragment) noexcept;

	inline const const_string& scheme() const noexcept {
		return scheme_;
	}

	uint16_t port() const noexcept {
		return port_;
	}

	inline const const_string& host() const noexcept {
		return host_;
	}

	inline const const_string& user_info() const noexcept {
		return user_info_;
	}

	inline const const_string& path() const noexcept {
		return path_;
	}

	inline const const_string& query() const noexcept {
		return query_;
	}

    inline const const_string& fragment() const noexcept {
    	return fragment_;
    }
private:
	uint16_t port_;
	const_string scheme_;
	const_string host_;
	const_string user_info_;
	const_string path_;
	const_string query_;
	const_string fragment_;
};


} // namespace net

} // namespace io

namespace std {

template<>
struct hash<io::net::uri> {
public:
	std::size_t operator()(const io::net::uri& u) noexcept
	{
		static constexpr std::size_t PRIME = 31;
		std::size_t ret = PRIME + u.scheme().hash();
		if( !u.host().empty() )
			ret = PRIME * ret + u.host().hash();
		if( !u.user_info().empty() )
			ret = PRIME * ret + u.user_info().hash();
		if( !u.path().empty() )
			ret = PRIME * ret + u.path().hash();
		if( !u.query().empty() )
			ret = PRIME * ret + u.query().hash();
		if( !u.fragment().empty() )
			ret = PRIME * ret + u.fragment().hash();
		ret = PRIME * ret + u.port();
		return ret;
	}
};

} // namespace std


#endif // __URI_HPP_INCLUDED__
