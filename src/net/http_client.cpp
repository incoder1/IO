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

namespace io {

namespace net {

namespace http {

// request
request::request(s_uri&& uri, headers&& hdrs) noexcept:
	uri_( std::forward<s_uri>(uri)),
	hdrs_( std::forward<headers>(hdrs) )
{}

void request::join(std::error_code& ec, byte_buffer& to) const noexcept
{
	to.put( uri_->path().data() );
	to.put(" HTTP/1.1\r\nHost: ");
	to.put( uri_->host().data() );
	to.put("\r\n", 2);
	typedef headers::const_iterator hit_t;
	typedef std::char_traits<char> ch8tr;
	std::size_t len;
	for(hit_t it = hdrs_.cbegin(); it != hdrs_.cend(); ++it) {
		len = ch8tr::length(it->first) + ch8tr::length(it->second) + 3;
		if( to.available() < len && !to.extend( (to.capacity() << 1) - to.capacity() ) ) {
			to.clear();
			ec = std::make_error_code(std::errc::not_enough_memory);
			return;
		}
		to.put(it->first);
		to.put(':');
		to.put(it->second);
		to.put("\r\n", 2);
	}
	to.put("\r\n", 2);
}

void request::send_all(std::error_code& ec, const s_write_channel& ch, byte_buffer& buff) const noexcept
{
	buff.flip();
	while( !ec && 0 != buff.length() )
		buff.move( ch->write(ec, buff.position().get(), buff.size() ) );
}

//get_request
class get_request final:public request {
private:
	static constexpr std::size_t INITIAL_BUFF_SIZE = 1024;
public:
	friend class nobadalloc<get_request>;

	get_request(const s_uri& uri, headers&& hdrs) noexcept:
		request( s_uri(uri), std::forward<headers>(hdrs) )
	{}

	virtual void send(std::error_code& ec, const s_write_channel& ch) const noexcept override
	{
		byte_buffer buff = byte_buffer::allocate(ec, INITIAL_BUFF_SIZE );
		if(!ec) {
			buff.put("GET ");
			join( ec, buff );
			if(!ec)
				send_all(ec, ch, buff);
		}
	}

};

s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,method m, const s_uri& resource, headers&& hdrs)
{
	switch( m ) {
	case method::get:
		return s_request(
		           nobadalloc<get_request>::construct(
		               ec,
		               resource,
		               std::forward<headers>(hdrs)
		           )
		       );
	default:
		ec = std::make_error_code(std::errc::not_supported);
		return s_request();
	}
}

} // namespace http

} // namespace net

} // namespace io

