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

static const char* SEP = "\r\n";

std::ostream& operator<<(std::ostream& to,const std::pair<const_string,const_string>& hdr) {
	to << hdr.first;
	to << ": ";
	to << hdr.second;
	to << SEP;
	return to;
}

// request
request::request(request_method method, const s_uri& uri) noexcept:
	method_(method),
	uri_( uri ),
	hdrs_()
{}

void request::send(std::error_code& ec, writer& to) const noexcept
{

	std::stringstream buff;
	switch(method_) {
		case request_method::get:
			buff << "GET ";
			break;
		case request_method::post:
			buff << "POST ";
			break;
		default:
			break;
	}
	buff << uri_->path();
	buff << " HTTP/1.1" << SEP ;
	buff << "Host: " << uri_->host() << SEP;
	for(auto hdr: hdrs_) {
		buff << hdr;
	}
	buff << SEP;
	to.write( ec,  buff.str() );
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

