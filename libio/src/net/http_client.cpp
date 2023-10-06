/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/net/http_client.hpp"

#include <sstream>

namespace io {

namespace net {

namespace http {

static const char* SEP = "\r\n";

static void write_header(writer& to,const std::pair<const_string,const_string>& hdr) {
	to.write(hdr.first.data(), hdr.first.size());
	to.write(": ");
	to.write(hdr.second.data(), hdr.second.size());
	to.write(SEP);
}

// request
request::request(request_method method, const s_uri& uri) noexcept:
	method_(method),
	uri_( uri ),
	hdrs_()
{}

void request::send(std::error_code& ec, writer& to) const noexcept
{
	// FIXME: refactor text
	switch(method_) {
		case request_method::get:
			to.write("GET ");
			break;
		case request_method::post:
			to.write("POST ");
			break;
		default:
			break;
	}
	to.write(uri_->path().data(),uri_->path().size());
	to.write(" HTTP/1.1");
	to.write(SEP);
	to.write("Host: ");
	to.write(uri_->host().data(), uri_->host().size());
	to.write(SEP);
	for(auto hdr: hdrs_) {
		write_header(to,hdr);
	}
	to.write(SEP);
	to.flush();
	ec = to.last_error();
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

