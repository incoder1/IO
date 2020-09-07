/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_SOURCE_HPP_INCLUDED__
#define __IO_XML_SOURCE_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "xml_error.hpp"
#include "text.hpp"
#include "charsetdetector.hpp"

namespace io {

namespace xml {

class source;

DECLARE_IPTR(source);

/// XML source abstraction, to be used by parser
class IO_PUBLIC_SYMBOL source final:public object {
    source(const source&) = delete;
    source& operator=(const source&) = delete;
public:

	/// Create new XML source from a readable byte channel
	/// \param ec operation error code
	/// \param src source byte channel
	/// \return smart pointer source reference when no error code, otherwise an empty smart pointer
    static s_source create(std::error_code& ec,s_read_channel&& src) noexcept;

	/// Create new XML source from a readable byte channel
	/// \param ec operation error code
	/// \param src source byte channel
	/// \return smart pointer source reference when no error code, otherwise an empty smart pointer
    static s_source create(std::error_code& ec,const s_read_channel& src) noexcept {
        return create(ec, s_read_channel(src) );
    }

    /// Releases internally allocated resources
    virtual ~source() noexcept override;

    /// Returns next character or character component byte
    char next() noexcept;

    /// Reads characters into buffer until a legal or illegal characters in the stream, or EOF
    /// \param to byte buffer to read into
    /// \param ch stop character
    /// \param illegal an illegal character in the may occur in the stream
    void read_until_char(byte_buffer& to,const char ch,const char illegal) noexcept;

	/// Reads characters into buffer until a double characters in the stream, or EOF
    /// \param to byte buffer to read into
    /// \param ch double characters to lookup, i.e. -- or ]]
    void read_until_double_char(byte_buffer& to, const char ch) noexcept;

    /// Checks current state is end of stream
    /// \return whether end of stream
    inline bool eof() const noexcept {
        return error::ok != last_ && pos_ != end_;
    }

    /// Current XML source character row
    inline std::size_t row() const noexcept {
        return row_;
    }

    /// Current XML source character column
    inline std::size_t col() const noexcept {
        return col_;
    }

	/// Returns last operation error
	/// \return last operation error
    inline error last_error() const noexcept {
        return last_;
    }
private:
    static const std::size_t READ_BUFF_INITIAL_SIZE;
    static const std::size_t READ_BUFF_MAXIMAL_SIZE;
    static s_source open(std::error_code& ec, const s_read_channel& src, byte_buffer&& rb) noexcept;
    friend io::nobadalloc<source>;
    source(s_read_channel&& src, byte_buffer&& rb) noexcept;
    error read_more() noexcept;
    error charge() noexcept;
    inline bool fetch() noexcept;
    inline char normalize_line_endings(const char ch);
private:
    error last_;
    const char *pos_;
    const char *end_;
    std::size_t row_;
    std::size_t col_;
    s_read_channel src_;
    byte_buffer rb_;
    uint8_t mb_state_;
};

} // namespace xml

} // namesapce io

#endif // __IO_XML_SOURCE_HPP_INCLUDED__
