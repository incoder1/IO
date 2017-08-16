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
	ftp
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

	uri(protocol scheme,uint16_t port,
		const_string&& host,const_string&& absolute,
		const_string&& relative, const_string&& fragment) noexcept;

	protocol scheme() const noexcept {
		return scheme_;
	}

	uint16_t port() const noexcept {
		return port_;
	}

	inline const const_string& host() const noexcept {
		return host_;
	}

    inline const const_string& absolute() const noexcept {
    	return absolute_;
    }

    inline const const_string& relative() const noexcept {
    	return relative_;
    }

    inline const const_string& fragment() const noexcept {
    	return fragment_;
    }
private:
	uint16_t port_;
	protocol scheme_;
	const_string host_;
    const_string absolute_;
    const_string relative_;
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
		std::size_t ret = PRIME * u.host().hash();
		ret = PRIME * ret + u.absolute().hash();
		ret = PRIME * ret + u.relative().hash();
		ret = PRIME * ret + u.fragment().hash();
		ret = PRIME * ret + u.port();
		ret = PRIME * ret + static_cast<std::size_t>(u.scheme());
		return ret;
	}
};

} // namespace std


#endif // __URI_HPP_INCLUDED__
