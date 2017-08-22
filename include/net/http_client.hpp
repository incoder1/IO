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

typedef std::pair<const char*,const char*> header;

class default_headers
{
public:
	static constexpr header ACCEPT_HTML_AND_XML = {"Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"};
	static constexpr header IO_USER_AGNET = {"User-Agent", "io library"};
	static constexpr header ACCEPT_CHARSET = {"Accept-Charset","ISO-8859-1,utf-8;q=0.7,*;q=0.7"};
};

enum class method
{
	get,
	post
	// TODO: add all
};

typedef std::vector<header> headers;

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

s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,method m, const s_uri& resource, headers&& hdrs);

} // namespace http


} // namespace net

} // namespace io


#endif // __IO_HTTP_CLIENT_HPP_INCLUDED__
