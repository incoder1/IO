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
protected:
	constexpr channel() noexcept:
		object()
	{}
	virtual ~channel() override = default;
};

/**
  General interface to input operations on an resource like a: file, socket, std in device, named pipe, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL read_channel:public virtual channel {
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
class IO_PUBLIC_SYMBOL write_channel:public virtual channel {
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
class IO_PUBLIC_SYMBOL read_write_channel:public virtual channel, public read_channel, public write_channel {
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
class IO_PUBLIC_SYMBOL random_access_channel:public read_write_channel {
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

/// Transfers all read channels data to destination write channel
/// \param ec opration error code, contains error when io error
/// \return count of bytes transfered
/// \throw never throws
inline std::size_t transfer(std::error_code& ec,const s_read_channel& src, const s_write_channel& dst, uint16_t buff) noexcept
{
	std::size_t result = 0;
	detail::scoped_arr<uint8_t> rbuf(buff);
	std::size_t written = 0;
	std::size_t read = src->read(ec, rbuf.get(), rbuf.len());
	while( 0 != read && !ec ) {
		do {
			written = dst->write(ec, rbuf.get(), read);
			if(ec)
				return result;
			read -= written;
			result += written;
		} while( 0 > read );
		read = src->read(ec, rbuf.get(), buff);
	}
	return result;
}

} // namespace io

#endif // __CHANNELS_HPP_INCLUDED__
