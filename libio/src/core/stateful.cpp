/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/core/stateful.hpp"

namespace io {

// pump
pump::pump() noexcept:
    object()
{}

bool pump::sync(std::error_code& ec) noexcept
{
    return true;
}

// channel_pump
channel_pump::channel_pump(s_read_channel&& src) noexcept:
      pump(),
      src_( std::forward<s_read_channel>(src) )
{}

s_pump channel_pump::create(std::error_code& ec, s_read_channel&& src) noexcept
{
    s_pump ret;
    channel_pump* px = new (std::nothrow) channel_pump( std::forward<s_read_channel>(src) );
    if(nullptr == px)
        ec = std::make_error_code(std::errc::not_enough_memory);
    else
        ret.reset(px, true);
    return ret;
}

std::size_t channel_pump::pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept
{
    return src_->read(ec, to, bytes);
}

// buffered_channel_pump
buffered_channel_pump::buffered_channel_pump(s_read_channel&& src,byte_buffer&& buff) noexcept:
    channel_pump( std::forward<s_read_channel>(src) ),
    read_buff_( std::forward<byte_buffer>(buff) )
{}

s_pump buffered_channel_pump::create(std::error_code& ec, s_read_channel&& src, std::size_t buffer_size) noexcept
{
    s_pump ret;
    byte_buffer buff = byte_buffer::allocate(ec,buffer_size);
    if(!ec) {
        uint8_t* px = const_cast<uint8_t*>(buff.position().get());
        std::size_t read = src->read(ec, px, buff.capacity());
        if(!ec && read > 0) {
            buff.move(read);
            buff.flip();
            auto px = new (std::nothrow) buffered_channel_pump( std::forward<s_read_channel>(src), std::move(buff) );
            if(nullptr == px)
                ec = std::make_error_code(std::errc::not_enough_memory);
            else
                ret.reset(px, true);
        }
    }
    return ret;
}

bool buffered_channel_pump::sync(std::error_code& ec) noexcept
{
    bool ret = false;
    std::size_t available = read_buff_.available();
    if(available > 0) {
        uint8_t* px = const_cast<uint8_t*>(read_buff_.position().get());
        std::size_t read = channel_pump::pull(ec, px, available);
        ret = !ec && read > 0;
        if(ret) {
            read_buff_.move(read);
            read_buff_.flip();
        }
    }
    return ret;
}

std::size_t buffered_channel_pump::pull(std::error_code& ec, uint8_t* const to,std::size_t bytes) noexcept
{
    std::size_t	ret = 0;
    while( (bytes > 0) && !ec) {
        std::size_t buffered = read_buff_.length();
        if( buffered >= bytes) {
            buffered = bytes;
        }
        else if( 0 == buffered) {
            read_buff_.clear();
            if( !sync(ec) )
                break;
        }
        io_memmove(to, read_buff_.position().get(), buffered );
        read_buff_.shift(buffered);
        ret += bytes;
        bytes -= buffered;
    }
    return ret;
}

// funnel
funnel::funnel() noexcept:
    object()
{}

void funnel::flush(std::error_code& ec) noexcept
{
}

// channel_funnel
channel_funnel::channel_funnel(s_write_channel&& dst) noexcept:
    funnel(),
    dst_( std::forward<s_write_channel>(dst) )
{}

s_funnel channel_funnel::create(std::error_code& ec, s_write_channel&& dst) noexcept
{
    s_funnel ret;
    channel_funnel* px = new (std::nothrow) channel_funnel( std::forward<s_write_channel>(dst) );
    if(nullptr == px)
        ec = std::make_error_code(std::errc::not_enough_memory);
    else
        ret.reset(px, true);
    return ret;
}

std::size_t channel_funnel::push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept
{
    return dst_->write(ec, src, bytes);
}

// buffered_channel_funnel
s_funnel buffered_channel_funnel::create(std::error_code& ec, s_write_channel&& dst, std::size_t buffer_size) noexcept
{
    s_funnel ret;
    byte_buffer buff = byte_buffer::allocate(ec, buffer_size);
    if(!ec) {
        buffered_channel_funnel* px = new (std::nothrow) buffered_channel_funnel(std::forward<s_write_channel>(dst), std::move(buff) );
        if(nullptr == px)
            ec = std::make_error_code(std::errc::not_enough_memory);
        else
            ret.reset(px, true);
    }
    return ret;
}

buffered_channel_funnel::buffered_channel_funnel(s_write_channel&& dst, byte_buffer&& buff) noexcept:
    channel_funnel(std::forward<s_write_channel>(dst)),
    write_buff_(std::forward<byte_buffer>(buff))
{}


void buffered_channel_funnel::flush(std::error_code& ec) noexcept
{
    write_buff_.flip();
    while(!ec && (write_buff_.length() > 0) ) {
        std::size_t flushed = channel_funnel::push(ec,  write_buff_.position().get(),  write_buff_.length());
        write_buff_.shift( flushed );
    }
    if(!ec)
        write_buff_.clear();
}

std::size_t buffered_channel_funnel::push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept
{
    std::size_t ret = 0;
    const uint8_t* px = src;
    while( !ec &&  (bytes > 0) ) {
        std::size_t available = write_buff_.available();
        if( available == 0 ) {
            flush(ec);
        }
        else if( bytes > write_buff_.capacity() ) {
            write_buff_.put(px, available);
            px += available;
            bytes -= available;
            ret += available;
        }
        else if( bytes <= available) {
            write_buff_.put( px, bytes );
            ret += bytes;
            bytes = 0;
        }
    }
    return ret;
}

} // namespace io
