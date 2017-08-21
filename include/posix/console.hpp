#ifndef __IO_POSIX_CONSOLE_HPP_INCLUDED__
#define __IO_POSIX_CONSOLE_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <memory>

#include <channels.hpp>
#include <stream.hpp>

#include "criticalsection.hpp"
#include "files.hpp"

namespace io
{

enum class text_color
{
    navy_blue,
    navy_green,
    navy_aqua,
    navy_red,
    magenta,
    brown,
    white,
    gray,
    light_blue,
    light_green,
    light_aqu,
    light_red,
    light_purple,
    yellow,
    bright_white
};

namespace posix
{

class IO_PUBLIC_SYMBOL console_channel final: public io::read_write_channel
{
    console_channel(const console_channel&) = delete;
    console_channel& operator=(const console_channel&) = delete;
public:
    console_channel(const fd_t stream) noexcept;
    virtual ~console_channel() = default;
    virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
    virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
    void change_color(text_color attr) noexcept;
private:
    fd_t stream_;
    const uint8_t* color_;
    std::size_t color_len_;
};

DECLARE_IPTR(console_channel);

} // namespace posix

class IO_PUBLIC_SYMBOL console
{
private:
    console();
    static const console* get();
    static void release_console() noexcept;
public:

    /// Reset default console output colors
    /// \param in color for input stream
    /// \param out color for output stream
    /// \param err color for error stream
    static void reset_colors(const text_color in,const text_color out,const text_color err) noexcept;

    /// Reset input stream console output color
    /// \param clr new color
    static void reset_in_color(const text_color clr) noexcept;

    /// Reset stdout stream console output color
    /// \param clr new color
    static void reset_out_color(const text_color clr) noexcept;

    /// Reset stderr stream console output color
    /// \param clr new color
    static void reset_err_color(const text_color crl) noexcept;

    /// Returns console input channel
    /// \return console input channel
    /// \throw can throw std::bad_alloc, when out of memory
    static inline s_read_channel in()
    {
        return s_read_channel( get()->in_ );
    }

    /// Returns console output channel
    /// \throw can throw std::bad_alloc, when out of memory
    static inline s_write_channel out()
    {
        return s_write_channel( get()->out_ );
    }

    static inline s_write_channel err()
    {
        return s_write_channel( get()->err_ );
    }

    /// Returns std::basic_stream<char> with ato-reconverting
    /// UTF-8 multibyte characters into console default UTF-16LE
    static std::ostream& out_stream();

    /// Returns std::basic_stream<char> with ato-reconverting
    /// UTF-8 multibyte characters into console default UTF-16LE
    static std::ostream& error_stream();

    /// Returns std::basic_stream<wchar_t> stream to constole output stream
    static std::wostream& out_wstream();

    /// Returns std::basic_stream<wchar_t> stream to constole error stream
    static std::wostream& error_wstream();

    /// Returns console character set, always UTF-16LE for Windows
    /// \return current console character set
    /// \throw never throws
    /* static inline const charset charset() noexcept
    {
        static charset __default = code_pages::UTF_8;
    	return __default;
    }
    */

private:
    posix::s_console_channel in_;
    posix::s_console_channel out_;
    posix::s_console_channel err_;
    static std::atomic<console*> _instance;
    static critical_section _cs;
};

} // namespace io

#endif // __IO_POSIX_CONSOLE_HPP_INCLUDED__
