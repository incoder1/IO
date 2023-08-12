/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_NET_HTTP_CLIENT_HPP_INCLUDED__
#define __IO_NET_HTTP_CLIENT_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <algorithm>
#include <map>
#include <utility>

#include <memory_channel.hpp>
#include <text.hpp>

#include "uri.hpp"

namespace io
{

namespace net
{

namespace http
{

enum class request_method
{
	connect,
	del,
	get,
	head,
	options,
	patch,
	post,
	put,
	trace
};

typedef std::pair<const const_string, const_string> header_t;
typedef std::map<const_string, const_string, std::less<const_string>, io::h_allocator<header_t> > headers_container_t;

class IO_PUBLIC_SYMBOL response:public object {
public:
	response() noexcept;
};

DECLARE_IPTR(response);


class IO_PUBLIC_SYMBOL request:public object
{
public:
	request(request_method method,const s_uri& uri) noexcept;
	void send(std::error_code& ec, writer& to) const noexcept;
	inline void add_header(header_t&& hdr) {
		hdrs_.emplace( hdr );
	}
	inline void add_header(const char* name, const char* value) noexcept
	{
		hdrs_.emplace(io::const_string(name),io::const_string(value));
	}
private:
	request_method method_;
	s_uri uri_;
	headers_container_t hdrs_;
};

DECLARE_IPTR(request);

// FIXME: refactor to client
s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,request_method m, const s_uri& resource) noexcept;

inline s_request new_get_request(std::error_code& ec, const s_uri& resource) noexcept
{
	s_request ret = new_request(ec, request_method::get, resource);
	if(!ec) {
		ret->add_header("User-Agent", "IO (C++ HTTP client lib)");
		ret->add_header("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
		ret->add_header("Accept-Language","en-US,en;q=0.5");
		ret->add_header("Connection","close");
		ret->add_header("Pragma", "no-cache");
		ret->add_header("Cache-Control", "no-cache");
	}
	return ret;
}


} // namespace http


} // namespace net

} // namespace io


#endif // __IO_NET_HTTP_CLIENT_HPP_INCLUDED__
