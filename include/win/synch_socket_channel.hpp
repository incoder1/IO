#ifndef __IO_WIN_SYNCH_SOCKET_CHANNEL_HPP_INCLUDED__
#define __IO_WIN_SYNCH_SOCKET_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "wsaerror.hpp"

namespace io {

namespace net {

class synch_socket_channel final:public read_write_channel {
private:
	friend class nobadalloc<synch_socket_channel>;
	synch_socket_channel(::SOCKET socket) noexcept:
		read_write_channel(),
		socket_(socket)
	{}
public:
	virtual ~synch_socket_channel() noexcept;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
public:
	::SOCKET socket_;
};

} // namespace net


} // namespace io



#endif // __IO_WIN_SYNCH_SOCKET_CHANNEL_HPP_INCLUDED__
