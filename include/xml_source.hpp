/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_SOURCE_HPP_INCLUDED__
#define __IO_XML_SOURCE_HPP_INCLUDED__

#include "channels.hpp"
#include "xml_error.hpp"
#include "text.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace xml {

class source;

DECLARE_IPTR(source);

class IO_PUBLIC_SYMBOL source final:public object
{
	source(const source&) = delete;
	source& operator=(const source&) = delete;
	public:
		static s_source create(std::error_code& ec, s_read_channel&& src, const charset& ch) noexcept;
		virtual ~source() noexcept override;
		char next() noexcept;
		inline std::size_t row() const noexcept {
			return row_;
		}
		inline std::size_t col() const noexcept {
			return col_;
		}
		inline error last_error() const noexcept {
			return last_;
		}
	private:
		static const std::size_t READ_BUFF_INITIAL_SIZE;
		static const std::size_t READ_BUFF_MAXIMAL_SIZE;
		static s_source create(std::error_code& ec, s_read_channel&& src, byte_buffer&& rb, const charset& ch) noexcept;
		source(s_read_channel&& src, byte_buffer&& rb) noexcept;
		error read_more() noexcept;
		error charge() noexcept;
		inline char normalize_lend(char ch);
		inline void new_line_or_shift_col(const char ch);
	private:
		s_read_channel src_;
		byte_buffer rb_;
		char *pos_;
		char *end_;
		std::size_t row_;
		std::size_t col_;
		uint8_t char_shift_;
		error last_;
};

} // namespace xml

} // namesapce io

#endif // __IO_XML_SOURCE_HPP_INCLUDED__
