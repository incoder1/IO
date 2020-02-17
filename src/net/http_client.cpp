/*
 *
 * Copyright (c) 2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "http_client.hpp"

#include <sstream>

namespace io {

namespace net {

namespace http {

inline const uint8_t* str_bytes(const char* str) noexcept {
	return reinterpret_cast<const uint8_t*>(str);
}

// request
request::request(request_method method, const s_uri& uri) noexcept:
	method_(method),
	uri_( uri ),
	hdrs_()
{}

void request::send(std::error_code& ec, const s_write_channel& to) const
{
	std::stringstream buff;
	switch(method_) {
		case request_method::get:
		default:
			buff << "GET ";
			break;
	}
	buff << uri_->path();
	buff << " HTTP/1.1\r\n";
	buff << "Host: " << uri_->host() << "\r\n";
	for(auto hdr: hdrs_) {
		buff << hdr.name() << ": ";
		buff << hdr.value() << "\r\n";
	}
	buff << "\r\n";
	std::string tmp = buff.str();
	to->write(ec, str_bytes( tmp.data() ), tmp.size() );
}

// FIXME: refactor to factory
s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,request_method m, const s_uri& resource) noexcept
{
	request *ret = new (std::nothrow) request(m, resource);
	if(nullptr == ret) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return s_request();
	}
	return s_request( ret );
}

} // namespace http

} // namespace net

} // namespace io

