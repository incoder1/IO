/*
 *
 * Copyright (c) 2016-2025
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

// channel_pump
channel_pump::channel_pump(const s_read_channel& src) noexcept:
      pump(),
      src_( src )
{}

s_pump channel_pump::create(std::error_code& ec, s_read_channel&& src) noexcept
{
    s_pump ret;
    channel_pump* px = new (std::nothrow) channel_pump( s_read_channel(src) );
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

s_pump buffered_channel_pump::create(std::error_code& ec,const s_read_channel& src, byte_buffer&& buff) noexcept
{
    s_pump ret;
    auto px = new (std::nothrow) buffered_channel_pump( src, std::forward<byte_buffer>(buff) );
    if(nullptr == px)
        ec = std::make_error_code(std::errc::not_enough_memory);
    else
        ret.reset(px, true);
    return ret;
}

s_pump buffered_channel_pump::create(std::error_code& ec,const s_read_channel& src, std::size_t buffer_size) noexcept
{
    s_pump ret;
    byte_buffer buff = byte_buffer::allocate(ec, buffer_size);
    if(!ec)
        ret = create(ec, src, std::move(buff)  );
    return ret;
}

buffered_channel_pump::buffered_channel_pump(const s_read_channel& src,byte_buffer&& buff) noexcept:
    channel_pump( src ),
    read_buff_( std::forward<byte_buffer>(buff) ),
    mtx_()
{}

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

std::size_t buffered_channel_pump::get_chunk_size(std::size_t bytes) noexcept
{
    std::size_t buffered = read_buff_.length();
    return bytes > buffered ? buffered : bytes;
}

std::size_t buffered_channel_pump::take(uint8_t* const to, std::size_t bytes) noexcept
{
    std::size_t length = read_buff_.length();
    std::size_t ret = bytes > length ? length : bytes;
    io_memmove(to, read_buff_.position().get(), ret );
    read_buff_.shift(ret);
    return ret;
}

std::size_t buffered_channel_pump::pull(std::error_code& ec, uint8_t* const to,std::size_t bytes) noexcept
{
    lock_guard lock(mtx_);
    std::size_t	ret = 0;
    uint8_t* i = to;
    while( (bytes > 0) && !ec) {
        if (0 == read_buff_.length()) {
            read_buff_.clear();
            if( !sync(ec) )
                break;
        }
        std::size_t taken = take(i, get_chunk_size(bytes));
        i += taken;
        ret += taken;
        bytes -= taken;
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
channel_funnel::channel_funnel(const s_write_channel& dst) noexcept:
    funnel(),
    dst_( dst )
{}

s_funnel channel_funnel::create(std::error_code& ec,const s_write_channel& dst) noexcept
{
    s_funnel ret;
    channel_funnel* px = new (std::nothrow) channel_funnel( dst );
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
s_funnel buffered_channel_funnel::create(std::error_code& ec,const s_write_channel& dst, std::size_t buffer_size) noexcept
{
    s_funnel ret;
    byte_buffer buff = byte_buffer::allocate(ec, buffer_size);
    if(!ec) {
        buffered_channel_funnel* px = new (std::nothrow) buffered_channel_funnel(s_write_channel(dst), std::move(buff) );
        if(nullptr == px)
            ec = std::make_error_code(std::errc::not_enough_memory);
        else
            ret.reset(px, true);
    }
    return ret;
}

buffered_channel_funnel::buffered_channel_funnel(const s_write_channel& dst, byte_buffer&& buff) noexcept:
    channel_funnel(dst),
    write_buff_(std::forward<byte_buffer>(buff)),
    mtx_()
{}

void buffered_channel_funnel::flush(std::error_code& ec) noexcept
{
    lock_guard lock(mtx_);
    write_buff_.flip();
    while(!ec && (write_buff_.length() > 0) ) {
        std::size_t flushed = channel_funnel::push(ec,  write_buff_.position().get(),  write_buff_.length());
        write_buff_.shift( flushed );
    }
    if(!ec)
        write_buff_.clear();
}

std::size_t buffered_channel_funnel::put(const uint8_t* src, std::size_t bytes) noexcept
{
    std::size_t available = write_buff_.available();
    std::size_t ret = bytes > available ? available : bytes;
    write_buff_.put(src, ret);
    return ret;
}

std::size_t buffered_channel_funnel::push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept
{
    lock_guard lock(mtx_);
    std::size_t ret = 0;
    const uint8_t* px = src;
    while( !ec &&  (bytes > 0) ) {
        if( write_buff_.full() ) {
            flush(ec);
        }
        std::size_t written = put(px, bytes);
        px += written;
        ret += written;
        bytes -= written;
    }
    return ret;
}

} // namespace io
