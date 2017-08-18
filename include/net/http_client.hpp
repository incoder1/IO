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

typedef std::pair<const char*, const char*> header;

inline std::ostream& operator<<(std::ostream& to, const header& hdr) {
	return to << hdr.first << ":" << hdr.second << "\r\n";
}

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

template<method>
class request
{};

template<>
class request<method::get>
{
public:
	request(const s_uri& resouce):
		uri_(resouce),
		headers_()
	{}
	void add_header(const header& hdr) {
		headers_.emplace_back( hdr );
	}
	void add_header(header&& hdr) {
		headers_.emplace_back( std::forward<header>(hdr) );
	}
	void add_headers(std::initializer_list<header>&& headers)
	{
		for(auto it = headers.begin(); it != headers.end(); ++it)
			headers_.emplace_back( std::move( *it ) );
	}
	void to_buff(byte_buffer& buff) const noexcept {
			buff.put("GET ");
			buff.put( uri_->path().data() );
			buff.put(" HTTP/1.1\r\nHost: ");
			buff.put( uri_->host().data() );
			buff.put( "\r\n");
			for(auto it = headers_.cbegin(); it != headers_.cend(); ++it)
			{
				buff.put(it->first);
				buff.put(':');
				buff.put(it->second);
				buff.put("\r\n");
			}
			buff.put("\r\n");
	}

private:
	s_uri uri_;
	std::vector<header, h_allocator<header, memory_traits> > headers_;
};

class responce_parser
{
public:
	responce_parser(const s_read_channel& from) noexcept:
		from_(from)
	{}
    s_read_channel& data() noexcept
    {
    	return from_;
    }
private:
	void parse_headers() {
	}
	void resolve_redirect() {
	}
private:
	s_read_channel from_;
};

} // namespace http


} // namespace net

} // namespace io


#endif // __IO_HTTP_CLIENT_HPP_INCLUDED__
