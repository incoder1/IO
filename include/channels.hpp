/*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __CHANNELS_HPP_INCLUDED__
#define __CHANNELS_HPP_INCLUDED__

#include <atomic>
#include <system_error>

#include "config.hpp"
#include "object.hpp"
#include "buffer.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

/// \brief General input/output channel interface
/// \details Adds ability for put implementor into instrusive_ptr
class IO_PUBLIC_SYMBOL channel:public object {
public:
	channel(const channel&) = delete;
	channel& operator=(const channel&) = delete;
	virtual ~channel() noexcept = 0;
protected:
	channel() noexcept;
private:
    std::atomic_size_t ref_count_;
    inline friend void intrusive_ptr_add_ref(channel* const ch) noexcept {
    	ch->ref_count_.fetch_add(1, std::memory_order_relaxed);
    }
    inline friend void intrusive_ptr_release(channel* const ch) noexcept {
    	if(1 == ch->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
			std::atomic_thread_fence( std::memory_order_release);
			delete ch;
    	}
    }
};

/**
  General interface to input operations on an resource like a: file, socket, std in device, named pipe, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL read_channel:public virtual channel
{
protected:
	read_channel() noexcept;
public:
	/// Reads certain number of bytes from underlying resource
	/// \param ec
	///		operation error code
	/// \param buff
	///		memory buffer to store read data, must be continues and at least size bytes length
	/// \param bytes
	///		requested bytes to read
	/// \return number of bytes read or 0 if nothing read or EOF riched
	/// \throw never throws
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept = 0;
};

DECLARE_IPTR(read_channel);

/**
 * General interface to output operations on an resource
 * like a: file, socket, std out device, named pipe, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL write_channel:public virtual channel
{
protected:
	write_channel() noexcept;
public:
	/// Writes certain number of bytes to underlying resource
	/// \param ec
	///		operation error code
	/// \param buff
	///		memory buffer to storing data to write, must be continues and at least size bytes length
	/// \param size
	///		requested bytes to write
	/// \return number of bytes written or 0 if nothing written
	/// \throw never throws
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept = 0;
};

DECLARE_IPTR(write_channel);


/**
* General interface to input and output operations on an resource
* like a: file, socket, std in/out device, named pipe, shared memory blocks etc.
**/
class IO_PUBLIC_SYMBOL read_write_channel:public virtual channel, public read_channel, public write_channel
{
protected:
	read_write_channel() noexcept;
public:
	virtual ~read_write_channel() noexcept = 0;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept = 0;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept = 0;
};

DECLARE_IPTR(read_write_channel);

/**
 * General interface to input, output and position moving operations on an resource
 * like a: file, socket, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL random_access_channel:public read_write_channel
{
public:
	/// Moving forward current device position
	/// \param ec
	///		operation error code
	/// \param size
	///			moving offset
	virtual std::size_t forward(std::error_code& err,std::size_t size) noexcept = 0;
	/// Moving backward current device position
	/// \param ec
	///		operation error code
	/// \param size
	///			moving offset
	virtual std::size_t backward(std::error_code& err, std::size_t size) noexcept = 0;
	/// Moving current device position forward from the device starting position
	/// \param ec
	///		operation error code
	/// \param size
	///			moving offset
	virtual std::size_t from_begin(std::error_code& err, std::size_t size) noexcept = 0;
	/// Moving current device position backward from the device ending position
	/// \param ec
	///		operation error code
	/// \param size
	///			moving offset
	virtual std::size_t from_end(std::error_code& err, std::size_t size) noexcept = 0;
	/// Gets current device position as an offset from the starting device position
	/// \param ec
	///		operation error code
	virtual std::size_t position(std::error_code& err) noexcept = 0;
};

DECLARE_IPTR(random_access_channel);

/// Transfers all read channel data to destination write channel
/// \param ec opration error code, contains error when io error
/// \return count of bytes transfered
/// \throw never throws
inline std::size_t transfer(std::error_code& ec,const s_read_channel& src, const s_write_channel& dst, uint16_t buff) noexcept
{
	std::size_t result = 0;
	uint8_t* rbuf = static_cast<uint8_t*> ( io_alloca( buff ) );
	std::size_t written = 0;
	std::size_t read = src->read(ec, rbuf, buff);
	while( 0 != read && !ec ) {
		do {
			written = dst->write(ec, rbuf, read);
			if(ec)
				return result;
			read -= written;
			result += written;
			written = 0;
		} while( 0 != read );
		read = src->read(ec, rbuf, buff);
	}
	return result;
}

std::size_t IO_PUBLIC_SYMBOL read_some(const read_channel* ch,uint8_t* const buff, std::size_t bytes);
std::size_t IO_PUBLIC_SYMBOL write_some(const write_channel* ch,const uint8_t* buff, std::size_t size);

inline std::size_t read_some(const s_read_channel& ch, uint8_t* const buff, std::size_t bytes)
{
	return read_some(ch.get(), buff, bytes);
}

inline std::size_t write_some(const s_write_channel& ch,const uint8_t* buff, std::size_t size)
{
	return write_some( ch.get(), buff, size);
}

inline void read_some(const s_read_channel& ch, byte_buffer& buff) {
	std::size_t read = read_some( ch.get(), const_cast<uint8_t*>( buff.position().get() ), buff.available() );
	buff.move(read);
}

inline std::size_t write_some(const write_channel* ch,const byte_buffer& buff) {
	return write_some( ch, buff.position().get(), buff.size() );
}

inline std::size_t write_some(const write_channel* ch,byte_buffer&& buff) {
	return write_some( ch, buff.position().get(), buff.size() );
}

inline std::size_t write_some(const s_write_channel& ch,const byte_buffer& buff) {
	return write_some( ch.get(), buff );
}

} // namespace io

#endif // __CHANNELS_HPP_INCLUDED__
