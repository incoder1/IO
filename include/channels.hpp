/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __CHANNELS_HPP_INCLUDED__
#define __CHANNELS_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>
#include <type_traits>

#include "object.hpp"
#include "buffer.hpp"
#include "scoped_array.hpp"
#include "errorcheck.hpp"

namespace io {


template <class C>
class unsafe {
	unsafe(const unsafe&) = delete;
	unsafe& operator=(const unsafe&) = delete;
};

/**
  General interface to input operations on an resource like a: file, socket, std in device, named pipe, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL read_channel:public virtual object {
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

template <>
class unsafe<read_channel> {
public:
    explicit unsafe(s_read_channel&& ch) noexcept:
        ec_(),
        ch_( std::forward<s_read_channel>(ch) )
    {}
    explicit unsafe(const s_read_channel& ch) noexcept:
        ec_(),
        ch_(ch)
    {}
    std::size_t read(uint8_t* const buff, std::size_t bytes) const
    {
        std::size_t ret = ch_->read( ec_, buff, bytes);
        io::check_error_code( ec_ );
        return ret;
    }
private:
    mutable std::error_code ec_;
    s_read_channel ch_;
};

/**
 * General interface to output operations on an resource
 * like a: file, socket, std out device, named pipe, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL write_channel:public virtual object {
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

template <>
class unsafe<write_channel> {
public:
    explicit unsafe(s_write_channel&& ch) noexcept:
        ec_(),
        ch_( std::forward<s_write_channel>(ch) )
    {}
    explicit unsafe(const s_write_channel& ch) noexcept:
        ec_(),
        ch_(ch)
    {}
    std::size_t write(const uint8_t* buff,const std::size_t size) const
    {
        std::size_t ret = ch_->write(ec_, buff, size);
        io::check_error_code( ec_ );
        return ret;
    }
private:
    mutable std::error_code ec_;
    s_write_channel ch_;
};


/**
* General interface to input and output operations on an resource
* like a: file, socket, std in/out device, named pipe, shared memory blocks etc.
**/
class IO_PUBLIC_SYMBOL read_write_channel:public virtual object, public read_channel, public write_channel {
protected:
    read_write_channel() noexcept;
public:
    virtual ~read_write_channel() noexcept = 0;
    virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override = 0;
    virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override = 0;
};

DECLARE_IPTR(read_write_channel);

template <>
class unsafe<read_write_channel> {
public:
    explicit unsafe(s_read_write_channel&& ch) noexcept:
        ec_(),
        ch_( std::forward<s_read_write_channel>(ch) )
    {}
    explicit unsafe(const s_read_write_channel& ch) noexcept:
        ec_(),
        ch_(ch)
    {}
    std::size_t read(uint8_t* const buff,const std::size_t bytes) const
    {
        std::size_t ret = ch_->read( ec_, buff, bytes);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t write(const uint8_t* buff,const std::size_t size) const
    {
        std::size_t ret = ch_->write( ec_, buff, size);
        io::check_error_code( ec_ );
        return ret;
    }
private:
    mutable std::error_code ec_;
    s_read_write_channel ch_;
};

/**
 * General interface to input, output and position moving operations on an resource
 * like a: file, socket, shared memory blocks etc.
 **/
class IO_PUBLIC_SYMBOL random_access_channel:public read_write_channel {
protected:
    random_access_channel() noexcept;
public:
    virtual ~random_access_channel() noexcept = 0;
    /// Moves current device position forward
    /// \param ec
    ///		operation error code
    /// \param size
    ///			moving offset
    virtual std::size_t forward(std::error_code& err,std::size_t size) noexcept = 0;
    /// Moves current device position backward
    /// \param ec
    ///		operation error code
    /// \param size
    ///			moving offset
    virtual std::size_t backward(std::error_code& err, std::size_t size) noexcept = 0;
    /// Moves current device position forward by offset from device begin
    /// \param ec
    ///		operation error code
    /// \param size
    ///			moving offset
    virtual std::size_t from_begin(std::error_code& err, std::size_t size) noexcept = 0;
    /// Moves current device backward from device end
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

template <>
class unsafe<random_access_channel> {
public:
    explicit unsafe(s_random_access_channel&& ch) noexcept:
        ec_(),
        ch_( std::forward<s_random_access_channel>(ch) )
    {}
    explicit unsafe(const s_random_access_channel& ch) noexcept:
        ec_(),
        ch_( ch )
    {}
    std::size_t read(uint8_t* const buff, std::size_t bytes) const
    {
        std::size_t ret = ch_->read( ec_, buff, bytes);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t write(const uint8_t* buff,std::size_t size) const
    {
        std::size_t ret = ch_->write( ec_, buff, size);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t forward(std::size_t size)
    {
        std::size_t ret = ch_->forward(ec_, size);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t backward(std::size_t size)
    {
        std::size_t ret = ch_->forward(ec_, size);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t from_begin(std::size_t size)
    {
        std::size_t ret = ch_->from_begin(ec_, size);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t from_end(std::size_t size)
    {
        std::size_t ret = ch_->from_end(ec_, size);
        io::check_error_code( ec_ );
        return ret;
    }
    std::size_t position()
    {
        std::size_t ret = ch_->position(ec_);
        io::check_error_code( ec_ );
        return ret;
    }
private:
    mutable std::error_code ec_;
    s_random_access_channel ch_;
};

class asynch_channel;
DECLARE_IPTR(asynch_channel);

class io_context;
DECLARE_IPTR(io_context);

class asynch_io_context;
DECLARE_IPTR(asynch_io_context);


class IO_PUBLIC_SYMBOL asynch_completion_routine:public object {
    asynch_completion_routine(const asynch_completion_routine&) = delete;
    asynch_completion_routine& operator=(asynch_completion_routine&) = delete;
public:
    constexpr asynch_completion_routine() noexcept:
        object()
    {}
    virtual ~asynch_completion_routine() noexcept = default;
    /// Routine calls by asynch_channel instances for handling asynchronous read operation complete
    /// \param ec contains i/o error if any
    /// \param source source channel which calls non blocking read operation
    /// \param data recaived data
    virtual void received(std::error_code& ec, const s_asynch_channel& source, io::byte_buffer&& data) noexcept;
    /// Routine calls by asynch_channel instances for handling asynchronous write operation complete
    /// \param ec contains i/o error if any
    /// \param source source channel which calls non blocking read operation
    /// \param data sent data
    /// \param transferred transferred bytes in data
    virtual void sent(std::error_code& ec, const s_asynch_channel& source, io::byte_buffer&& data) noexcept;
};

DECLARE_IPTR(asynch_completion_routine);

class IO_PUBLIC_SYMBOL asynch_channel:public object {
    asynch_channel(const asynch_channel&) = delete;
    asynch_channel& operator=(const asynch_channel&) = delete;
protected:
    asynch_channel(const s_asynch_completion_routine& routines, const asynch_io_context* context) noexcept;
public:
    virtual ~asynch_channel() noexcept = 0;
    /// Returns current completion routine
    s_asynch_completion_routine routine() const noexcept
    {
        return routine_;
    }
    const asynch_io_context* context() const noexcept {
    	return context_;
    }
    virtual void recaive(std::error_code& ec, std::size_t amout, std::size_t position) const noexcept = 0;
    virtual void send(std::error_code& ec, byte_buffer&& what,std::size_t position) const noexcept = 0;
    virtual bool cancel_pending() const noexcept = 0;
    virtual bool cancel_all() const noexcept = 0;
private:
    s_asynch_completion_routine routine_;
    const asynch_io_context* context_;
};

DECLARE_IPTR(asynch_channel);

/// Transmits a buffer data into a write channel
/// function will re-attempt to write unless
/// size bytes from buffer will be written to the destination
/// channel, or an io_error
/// \param ec operation error code, contains error when io error
/// \param buff memory buffer, must not be nullptr
/// \param size count of bytes to transmit,
///				must be greater then 0 and less or equals to memory buffer size
/// \param dst destination write channel
std::size_t IO_PUBLIC_SYMBOL transmit_buffer(
    std::error_code& ec,
    const s_write_channel& dst,
    const uint8_t* buffer,
    std::size_t size) noexcept;

/// Transmits all read channels data to destination write channel
/// \param ec operation error code, contains error
///				when io error or not enough memory for allocating buffer
/// \param src source read channel
/// \param dst destination write channel
/// \param buff memory buffer size, OS page size will be used if 0
/// \return count of bytes transfered
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transmit(
	std::error_code& ec,
	const s_read_channel& src,
	const s_write_channel& dst,
	std::size_t buff_size) noexcept;

} // namespace io

#endif // __CHANNELS_HPP_INCLUDED__
