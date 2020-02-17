#ifndef __IO_HTTP_CLIENT_HPP_INCLUDED__
#define __IO_HTTP_CLIENT_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <algorithm>
#include <vector>
#include <utility>

#include <memory_channel.hpp>
#include "uri.hpp"

namespace io {

namespace net {

namespace http {

class header {
   public:
   		/*
        header(const const_string& name, const const_string& value) noexcept:
            name_(name),
            value_(value)
        {}
        */
        header(const_string&& name,const_string&& value) noexcept:
            name_( std::forward<const_string>(name) ),
            value_( std::forward<const_string>(value) )
        {}
		const_string name() const noexcept {
			return name_;
		}
		const_string value() const noexcept {
			return value_;
		}
   private:
       const_string name_;
       const_string value_;
};

enum class request_method
{
	get,
	post
	// TODO: add all
};

typedef std::vector<header> headers;


class IO_PUBLIC_SYMBOL request:public object
{
public:
	request(request_method method,const s_uri& uri) noexcept;
	void send(std::error_code& ec, const s_write_channel& ch) const;
	void add_header(const char* name, const char* value) {
		hdrs_.emplace_back( io::const_string(name), io::const_string(value) );
	}
private:
	request_method method_;
	s_uri uri_;
	headers hdrs_;
};

DECLARE_IPTR(request);

// FIXME: refactor to factory
s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,request_method m, const s_uri& resource) noexcept;

inline s_request new_get_request(std::error_code& ec, const s_uri& resource) noexcept
{
    s_request ret = new_request(ec, request_method::get, resource);
    if(!ec) {
		ret->add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:72.0) Gecko/20100101 Firefox/72.0");
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


#endif // __IO_HTTP_CLIENT_HPP_INCLUDED__
