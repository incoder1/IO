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

inline const uint8_t* u8cast(const char* str) noexcept {
	return reinterpret_cast<const uint8_t*>(str);
}

// request
request::request(s_uri&& uri, headers&& hdrs) noexcept:
	uri_( std::forward<s_uri>(uri)),
	hdrs_( std::forward<headers>(hdrs) )
{}

void request::join(std::error_code& ec, const s_write_channel& to) const noexcept
{
	static constexpr const char* PROTO = " HTTP/1.1\r\nHost: ";
	static const std::size_t PROTO_LEN = io_strlen(PROTO);
	static constexpr const char* DELIM = "\r\n";
	static const std::size_t DELIM_LEN = io_strlen(DELIM);
	const_string path  = uri_->path();
	to->write(ec, u8cast(path.data()), path.size() );
	to->write(ec, u8cast(PROTO), PROTO_LEN );
	const_string host = uri_->host();
	to->write(ec, u8cast(host.data()), host.size() );
	to->write(ec, u8cast(DELIM), DELIM_LEN);
	for(auto it = hdrs_.cbegin(); it != hdrs_.cend(); ++it) {
		const_string name = it->name();
		to->write(ec, u8cast(name.data()), name.size() );
		to->write(ec, u8cast(":"), 1);
		const_string value = it->value();
		to->write(ec, u8cast(value.data()), value.size() );
		to->write(ec, u8cast(DELIM), DELIM_LEN);
	}
	to->write(ec, u8cast(DELIM), DELIM_LEN);
}


//get_request
class get_request final:public request {
private:
	static constexpr std::size_t INITIAL_BUFF_SIZE = 1024;
public:
	friend class nobadalloc<get_request>;

	static s_request create(std::error_code& ec, const s_uri& resource, std::vector<header>&& hdrs) noexcept {
		get_request *ret = nobadalloc<get_request>::construct(ec, resource, std::forward< std::vector<header> >(hdrs) );
		return (nullptr != ret) ? s_request(ret) : s_request();
	}

	get_request(const s_uri& uri, headers&& hdrs) noexcept:
		request( s_uri(uri), std::forward<headers>(hdrs) )
	{}

	virtual ~get_request() noexcept override
	{}

	virtual void send(std::error_code& ec, const s_write_channel& ch) const noexcept override {
        static constexpr const char* TYPE = "GET ";
        static const std::size_t TYPE_LEN = io_strlen("GET ");
        ch->write(ec, u8cast(TYPE), TYPE_LEN);
        this->join(ec, ch);
	}

};

/*
class post_request final:public request {
};
*/

// FIXME: refactor to factory
s_request IO_PUBLIC_SYMBOL new_request(std::error_code& ec,method m, const s_uri& resource, std::vector<header>&& hdrs) noexcept
{
	switch(m) {
		case method::get:
			return get_request::create(ec, resource, std::forward< std::vector<header> >(hdrs) );
		default:
			break;
	}
	ec = std::make_error_code(std::errc::not_supported);
	return s_request();
}

} // namespace http

} // namespace net

} // namespace io

