#ifndef __IO_WIN_SSPI_CHANNEL_HPP_INCLUDED__
#define __IO_WIN_SSPI_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

// stdlib C++ headers
#include <atomic>

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32 1
#endif // SECURITY_WIN32

// windows headers
#include <security.h>
#include <sspi.h>
#include <rpc.h>
#include <schnlsp.h>
#include <schannel.h>

// io headers
#include <channels.hpp>
#include "sockets.hpp"
#include "criticalsection.hpp"

namespace io {

namespace net {

namespace secure {

struct credentials {
	credentials(const credentials&)  = delete;
	credentials& operator=(const credentials&)  = delete;
public:
	constexpr explicit credentials() noexcept:
		h_cred_({0,0})
	{}
	credentials(credentials&& rhs) noexcept:
		h_cred_( std::move(rhs.h_cred_) )
	{
		rhs.h_cred_ = {0,0};
	}
	credentials& operator=(credentials&& rhs) noexcept {
		credentials( std::forward<credentials>(rhs) ).swap( *this );
		return *this;
	}
	explicit credentials(::CredHandle&& hnd) noexcept:
		h_cred_( std::move(hnd) )
	{}
	::PCredHandle get() const noexcept
	{
        return const_cast<::PCredHandle>(&h_cred_);
	}
	~credentials() noexcept {
		if( check(h_cred_) )
			::FreeCredentialsHandle(&h_cred_);
	}
private:
	void swap(credentials& other) noexcept {
		::CredHandle tmp = {h_cred_.dwLower, h_cred_.dwUpper};
		h_cred_ = {other.h_cred_.dwLower,h_cred_.dwUpper};
		other.h_cred_ = {tmp.dwLower, tmp.dwUpper};
	}
	static inline constexpr bool check(const ::CredHandle& hnd) {
		return hnd.dwLower !=  0 &&  hnd.dwUpper != 0;
	}
private:
	::CredHandle h_cred_;
};

class connection_contex final
{
	connection_contex(const connection_contex&) = delete;
	connection_contex& operator=(const connection_contex&) = delete;
public:
	static constexpr std::size_t MAX_MESSAGE = 12000;
	constexpr explicit connection_contex() noexcept:
		in_buff_(),
		out_buff_(),
		socket_()
	{}
	connection_contex(connection_contex&& mv) noexcept:
		in_buff_( std::move(mv.in_buff_) ),
		out_buff_( std::move(mv.out_buff_) ),
		socket_( std::move(mv.socket_) )
	{}

	connection_contex& operator=(connection_contex&& rhs) noexcept
	{
		connection_contex( std::forward<connection_contex>(rhs) ).swap( *this );
		return *this;
	}

	connection_contex(scoped_arr<uint8_t>&& in, scoped_arr<uint8_t>&& out, s_read_write_channel&& sock) noexcept:
		in_buff_( std::forward<scoped_arr<uint8_t> >(in) ),
		out_buff_( std::forward<scoped_arr<uint8_t> >(out) ),
		socket_( std::forward<s_read_write_channel>(sock) )
	{}

private:

	void swap(connection_contex& other) noexcept
	{
		in_buff_.swap( other.in_buff_ );
		out_buff_.swap( other.out_buff_ );
		socket_.swap( other.socket_ );
	}

private:
	io::scoped_arr<uint8_t> in_buff_;
	io::scoped_arr<uint8_t> out_buff_;
	s_read_write_channel socket_;
};


class IO_PUBLIC_SYMBOL service final {
	service(const service&) = delete;
	service& operator=(const service&) = delete;
public:
	static const service* instance(std::error_code& ec) noexcept;
	~service() noexcept;
	s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:


	connection_contex client_handshake(std::error_code& ec, s_read_write_channel&& socket) noexcept;

	static credentials sspi_init(std::error_code& ec) noexcept;

	static void release_on_exit() noexcept;

	explicit service(credentials&& creds) noexcept;
	static std::atomic<service*> _instance;
	static critical_section _init_mtx;

	credentials creds_;
};

/*
class  sspi_channel
{
	public:
		sspi_channel();
		virtual ~sspi_channel();
	private:
};
*/

} // secure

} // net

} // namespace io

#endif // __IO_WIN_SSPI_CHANNEL_HPP_INCLUDED__
