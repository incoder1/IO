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
#include <conststring.hpp>
#include <stringpool.hpp>

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
	request(const cached_string& path, const cached_string& host):
		path_(path),
		host_(host),
		headers_()
	{}
	void add_headers(std::initializer_list<header>&& headers)
	{
		for(auto it = headers.begin(); it != headers.end(); ++it)
			headers_.emplace_back( std::move( *it ) );
	}
	void to_buff(byte_buffer& buff) const noexcept {
			buff.put("GET ");
			buff.put( path_.data() );
			buff.put(" HTTP/1.1\r\nHost: ");
			buff.put( host_.data() );
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
	cached_string path_;
	cached_string host_;
	std::vector<header, h_allocator<header, memory_traits> > headers_;
};

} // namespace http

namespace http2 {



} // namespace http2

} // namespace net

} // namespace io


#endif // __IO_HTTP_CLIENT_HPP_INCLUDED__
