#ifndef __IO_HTTP_CLIENT_HPP_INCLUDED__
#define __IO_HTTP_CLIENT_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <algorithm>
#include <vector>
#include <utility>

#include <buffer.hpp>

#include "uri.hpp"

namespace io {

namespace net {

namespace http {

class header {
   public:
        header(const const_string& name, const const_string& value) noexcept:
            name_(name),
            value_(value)
        {}
        header(const_string&& name,const_string&& value) noexcept:
            name_( std::forward<const_string>(name) ),
            value_( std::forward<const_string>(value) )
        {}
        bool to_bytes(byte_buffer& buff) const noexcept
        {
            return buff.put( name_.data() ) && buff.put(':') && buff.put( value_.data() ) && buff.put("\r\n", 2);
        }
   private:
       const_string name_;
       const_string value_;
};

enum class method
{
	get,
	post
	// TODO: add all
};

typedef scoped_arr<header> headers;

class IO_PUBLIC_SYMBOL request:public virtual object
{
protected:
	request(s_uri&& uri, headers&& hdrs) noexcept;
	void join(std::error_code& ec, byte_buffer& to) const noexcept;
	void send_all(std::error_code& ec, const s_write_channel& ch, byte_buffer& buff) const noexcept;
public:
	virtual void send(std::error_code& ec, const s_write_channel& ch) const noexcept = 0;
private:
	s_uri uri_;
	headers hdrs_;
};

DECLARE_IPTR(request);

s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,method m, const s_uri& resource, std::initializer_list<header>&& hdrs) noexcept;

inline s_request new_request(std::error_code& ec,method m, const s_uri& resource) noexcept
{
    return new_request(ec, m, resource,
            {
				{"Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
				{"User-Agent", "io library"},
				{"Accept-Charset","ISO-8859-1,utf-8;q=0.7,*;q=0.7"},
				{"Connection"," close"}
			} );
}



} // namespace http


} // namespace net

} // namespace io


#endif // __IO_HTTP_CLIENT_HPP_INCLUDED__
